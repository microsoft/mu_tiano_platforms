/** @file

  ArmPlatformGetPlatformPpiList for QEMU/arm-virt: publishes CPU topology
  via ARM_MP_CORE_INFO_PPI by walking /cpus/cpu@N in the DTB.

  This library runs XIP (no writable .data), so each
  PrePeiCoreGetMpCoreInfo() call re-walks the DTB and allocates a fresh
  ARM_CORE_INFO table; the PPI caller copies it into a HOB.

  Copyright (c) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Uefi.h>

#include <Library/ArmLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/FdtLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

#include <Ppi/ArmMpCoreInfo.h>

//
// QEMU virt's DTB encodes the MPIDR of each CPU in the `reg` property of the
// /cpus/cpu@N node:
//   - 1-cell form: low 24 bits of MPIDR_EL1 (Aff0/Aff1/Aff2 packed); Aff3 = 0.
//   - 2-cell form: high cell holds Aff3 in bits[7:0]; low cell holds
//                  Aff0/Aff1/Aff2 packed as above.
// In both cases, the resulting 64-bit value matches MPIDR_EL1's affinity
// layout (Aff0=[7:0], Aff1=[15:8], Aff2=[23:16], Aff3=[39:32]).
//

/**
  Decode a single `cpu@N/reg` property into an MPIDR_EL1 affinity value.

  @param[in]  RegData     Raw `reg` property bytes (big-endian, FDT-encoded).
  @param[in]  RegLen      Length in bytes of RegData.
  @param[in]  CellCount   Number of 32-bit cells per `reg` entry (from the
                          parent /cpus node's `#address-cells`).
  @param[out] Mpidr       On success, the decoded MPIDR_EL1 affinity value.

  @retval EFI_SUCCESS            The MPIDR was decoded successfully.
  @retval EFI_INVALID_PARAMETER  The `reg` length is too small for CellCount,
                                 or CellCount is unsupported.
**/
STATIC
EFI_STATUS
DecodeCpuRegToMpidr (
  IN  CONST UINT8  *RegData,
  IN  INT32        RegLen,
  IN  INT32        CellCount,
  OUT UINT64       *Mpidr
  )
{
  UINT32  LowCell;
  UINT32  HighCell;

  if ((RegData == NULL) || (Mpidr == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((CellCount != 1) && (CellCount != 2)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Unsupported /cpus #address-cells = %d\n",
      __func__,
      CellCount
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (RegLen < (INT32)(CellCount * sizeof (UINT32))) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: cpu@N/reg too short (%d bytes, need %d)\n",
      __func__,
      RegLen,
      CellCount * (INT32)sizeof (UINT32)
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (CellCount == 1) {
    LowCell = Fdt32ToCpu (ReadUnaligned32 ((CONST UINT32 *)RegData));
    *Mpidr  = (UINT64)LowCell;
  } else {
    HighCell = Fdt32ToCpu (ReadUnaligned32 ((CONST UINT32 *)RegData));
    LowCell  = Fdt32ToCpu (ReadUnaligned32 ((CONST UINT32 *)RegData + 1));
    *Mpidr   = ((UINT64)HighCell << 32) | (UINT64)LowCell;
  }

  return EFI_SUCCESS;
}

/**
  Count the number of `device_type = "cpu"` child nodes under /cpus in the
  given DTB.

  @param[in]  Fdt          Pointer to a validated FDT blob.
  @param[in]  CpusOffset   Offset of the /cpus node in the FDT.
  @param[out] CpuCount     On success, the number of CPU child nodes found.

  @retval EFI_SUCCESS       At least one CPU was found.
  @retval EFI_NOT_FOUND     No CPU child nodes were found.
**/
STATIC
EFI_STATUS
CountCpuNodes (
  IN  CONST VOID  *Fdt,
  IN  INT32       CpusOffset,
  OUT UINT32      *CpuCount
  )
{
  INT32               Node;
  INT32               Len;
  CONST FDT_PROPERTY  *PropertyPtr;
  UINT32              Count;

  Count = 0;
  for (Node = FdtFirstSubnode (Fdt, CpusOffset);
       Node >= 0;
       Node = FdtNextSubnode (Fdt, Node))
  {
    PropertyPtr = FdtGetProperty (Fdt, Node, "device_type", &Len);
    if ((PropertyPtr == NULL) || (PropertyPtr->Data == NULL)) {
      continue;
    }

    if (AsciiStrnCmp ((CONST CHAR8 *)PropertyPtr->Data, "cpu", Len) != 0) {
      continue;
    }

    Count++;
  }

  if (Count == 0) {
    return EFI_NOT_FOUND;
  }

  *CpuCount = Count;
  return EFI_SUCCESS;
}

/**
  Build the per-CPU ARM_CORE_INFO table dynamically by enumerating
  /cpus/cpu@N nodes in the DTB.

  A fresh table is allocated on every call (this library runs XIP and cannot
  use mutable globals to cache state). The caller of the
  ARM_MP_CORE_INFO_PPI copies the contents into a HOB, so the buffer is only
  required to outlive the GetMpCoreInfo() invocation.

  @param[out] CoreCount     Number of entries written into the returned table.
  @param[out] ArmCoreTable  Pointer to the freshly allocated ARM_CORE_INFO
                            table.

  @retval EFI_SUCCESS            Table successfully built.
  @retval EFI_NOT_FOUND          No /cpus or no cpu@N nodes were found.
  @retval EFI_DEVICE_ERROR       The DTB pointer or contents are invalid.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate memory for the table.
**/
STATIC
EFI_STATUS
BuildArmPlatformMpCoreInfoTable (
  OUT UINTN          *CoreCount,
  OUT ARM_CORE_INFO  **ArmCoreTable
  )
{
  EFI_STATUS          Status;
  CONST VOID          *Fdt;
  INT32               CpusOffset;
  INT32               AddressCells;
  INT32               Node;
  INT32               Len;
  CONST FDT_PROPERTY  *PropertyPtr;
  UINT32              NumCpus;
  UINT32              Index;
  ARM_CORE_INFO       *Table;

  Fdt = (CONST VOID *)(UINTN)PcdGet64 (PcdDeviceTreeInitialBaseAddress);
  if ((Fdt == NULL) || (FdtCheckHeader (Fdt) != 0)) {
    DEBUG ((DEBUG_ERROR, "%a: Invalid or missing device tree blob\n", __func__));
    return EFI_DEVICE_ERROR;
  }

  CpusOffset = FdtPathOffset (Fdt, "/cpus");
  if (CpusOffset < 0) {
    DEBUG ((DEBUG_ERROR, "%a: /cpus node not found in DTB\n", __func__));
    return EFI_NOT_FOUND;
  }

  AddressCells = FdtAddressCells (Fdt, CpusOffset);
  if ((AddressCells != 1) && (AddressCells != 2)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: /cpus #address-cells = %d (expected 1 or 2)\n",
      __func__,
      AddressCells
      ));
    return EFI_DEVICE_ERROR;
  }

  Status = CountCpuNodes (Fdt, CpusOffset, &NumCpus);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: No cpu@N nodes found under /cpus\n", __func__));
    return Status;
  }

  Table = AllocateZeroPool (sizeof (ARM_CORE_INFO) * NumCpus);
  if (Table == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Failed to allocate ARM_CORE_INFO table for %u CPUs\n",
      __func__,
      NumCpus
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  Index = 0;
  for (Node = FdtFirstSubnode (Fdt, CpusOffset);
       Node >= 0;
       Node = FdtNextSubnode (Fdt, Node))
  {
    PropertyPtr = FdtGetProperty (Fdt, Node, "device_type", &Len);
    if ((PropertyPtr == NULL) || (PropertyPtr->Data == NULL)) {
      continue;
    }

    if (AsciiStrnCmp ((CONST CHAR8 *)PropertyPtr->Data, "cpu", Len) != 0) {
      continue;
    }

    PropertyPtr = FdtGetProperty (Fdt, Node, "reg", &Len);
    if (PropertyPtr == NULL) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: cpu node at index %u has no 'reg' property\n",
        __func__,
        Index
        ));
      FreePool (Table);
      return EFI_DEVICE_ERROR;
    }

    Status = DecodeCpuRegToMpidr (
               (CONST UINT8 *)PropertyPtr->Data,
               Len,
               AddressCells,
               &Table[Index].Mpidr
               );
    if (EFI_ERROR (Status)) {
      FreePool (Table);
      return Status;
    }

    //
    // QEMU virt releases secondary cores via PSCI; there is no poll-based
    // mailbox to program. Mark the mailbox addresses as unused.
    //
    Table[Index].MailboxSetAddress   = (EFI_PHYSICAL_ADDRESS)0;
    Table[Index].MailboxGetAddress   = (EFI_PHYSICAL_ADDRESS)0;
    Table[Index].MailboxClearAddress = (EFI_PHYSICAL_ADDRESS)0;
    Table[Index].MailboxClearValue   = (UINT64)0xFFFFFFFF;

    DEBUG ((
      DEBUG_INFO,
      "%a: CPU[%u] MPIDR = 0x%lx\n",
      __func__,
      Index,
      Table[Index].Mpidr
      ));

    Index++;
  }

  if (Index != NumCpus) {
    //
    // Belt-and-braces: the second pass should match the count from the first
    // pass. If it doesn't, something concurrent (or a buggy FDT) is at play.
    //
    DEBUG ((
      DEBUG_ERROR,
      "%a: CPU count mismatch (counted %u, walked %u)\n",
      __func__,
      NumCpus,
      Index
      ));
    FreePool (Table);
    return EFI_DEVICE_ERROR;
  }

  *CoreCount    = NumCpus;
  *ArmCoreTable = Table;
  return EFI_SUCCESS;
}

/**
  ARM_MP_CORE_INFO_PPI implementation for QEMU virt.

  @param[out] CoreCount     Number of CPUs reported.
  @param[out] ArmCoreTable  Allocated ARM_CORE_INFO table; ownership is
                            transferred to the caller.

  @retval EFI_UNSUPPORTED   Running on a uniprocessor system.
  @retval Others            See BuildArmPlatformMpCoreInfoTable().
**/
EFI_STATUS
EFIAPI
PrePeiCoreGetMpCoreInfo (
  OUT UINTN          *CoreCount,
  OUT ARM_CORE_INFO  **ArmCoreTable
  )
{
  if (!ArmIsMpCore ()) {
    return EFI_UNSUPPORTED;
  }

  return BuildArmPlatformMpCoreInfoTable (CoreCount, ArmCoreTable);
}

STATIC ARM_MP_CORE_INFO_PPI  mMpCoreInfoPpi = { PrePeiCoreGetMpCoreInfo };

STATIC EFI_PEI_PPI_DESCRIPTOR  gPlatformPpiTable[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gArmMpCoreInfoPpiGuid,
    &mMpCoreInfoPpi
  }
};

/**
  Return the Platform specific PPIs.

  This function exposes the Platform Specific PPIs. They can be used by any
  PrePi modules or passed to the PeiCore by PrePeiCore.

  On a uniprocessor system, return an empty list (no MP core info PPI is
  needed). On an MP system, advertise the ARM_MP_CORE_INFO_PPI backed by
  PrePeiCoreGetMpCoreInfo() above.

  @param[out]   PpiListSize         Size in Bytes of the Platform PPI List.
  @param[out]   PpiList             Platform PPI List.
**/
VOID
ArmPlatformGetPlatformPpiList (
  OUT UINTN                   *PpiListSize,
  OUT EFI_PEI_PPI_DESCRIPTOR  **PpiList
  )
{
  if (ArmIsMpCore ()) {
    *PpiListSize = sizeof (gPlatformPpiTable);
    *PpiList     = gPlatformPpiTable;
  } else {
    *PpiListSize = 0;
    *PpiList     = NULL;
  }
}
