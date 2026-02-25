/** @file
*  FdtHelperLib.c
*
*  Copyright (c) 2021, NUVIA Inc. All rights reserved.
*  Copyright (c) 2020, Linaro Ltd. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/FdtHelperLib.h>
#include <Library/PcdLib.h>
#include <Library/FdtLib.h>

STATIC INT32  mFdtFirstCpuOffset;
STATIC INT32  mFdtCpuNodeSize;

/**
  Get MPIDR for a given cpu from device tree passed by Qemu.

  @param [in]   CpuId    Index of cpu to retrieve MPIDR value for.

  @retval                MPIDR value of CPU at index <CpuId>
**/
UINT64
FdtHelperGetMpidr (
  IN UINTN  CpuId
  )
{
  VOID                *DeviceTreeBase;
  CONST FDT_PROPERTY  *PropertyPtr;
  INT32               Len;

  DeviceTreeBase = (VOID *)(UINTN)PcdGet64 (PcdDeviceTreeInitialBaseAddress);
  ASSERT (DeviceTreeBase != NULL);

  PropertyPtr = FdtGetProperty (
                  DeviceTreeBase,
                  mFdtFirstCpuOffset + (CpuId * mFdtCpuNodeSize),
                  "reg",
                  &Len
                  );
  if (!PropertyPtr) {
    DEBUG ((DEBUG_ERROR, "Couldn't find reg property for CPU:%d\n", CpuId));
    return 0;
  }

  return (Fdt64ToCpu (ReadUnaligned64 ((UINT64 *)PropertyPtr->Data)));
}

/** Walks through the Device Tree created by Qemu and counts the number
    of CPUs present in it.

    @return The number of CPUs present.
**/
EFIAPI
UINT32
FdtHelperCountCpus (
  VOID
  )
{
  VOID    *DeviceTreeBase;
  INT32   Node;
  INT32   Prev;
  INT32   CpuNode;
  UINT32  CpuCount;

  DeviceTreeBase = (VOID *)(UINTN)PcdGet64 (PcdDeviceTreeInitialBaseAddress);
  ASSERT (DeviceTreeBase != NULL);

  // Make sure we have a valid device tree blob
  ASSERT (FdtCheckHeader (DeviceTreeBase) == 0);

  CpuNode = FdtPathOffset (DeviceTreeBase, "/cpus");
  if (CpuNode <= 0) {
    DEBUG ((DEBUG_ERROR, "Unable to locate /cpus in device tree\n"));
    return 0;
  }

  CpuCount = 0;

  // Walk through /cpus node and count the number of subnodes.
  // The count of these subnodes corresponds to the number of
  // CPUs created by Qemu.
  Prev               = FdtFirstSubnode (DeviceTreeBase, CpuNode);
  mFdtFirstCpuOffset = Prev;
  while (1) {
    CpuCount++;
    Node = FdtNextSubnode (DeviceTreeBase, Prev);
    if (Node < 0) {
      break;
    }

    mFdtCpuNodeSize = Node - Prev;
    Prev            = Node;
  }

  return CpuCount;
}
