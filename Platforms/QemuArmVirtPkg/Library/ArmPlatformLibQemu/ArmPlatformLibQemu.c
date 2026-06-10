/** @file

  Copyright (c) 2011-2012, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Uefi.h>
#include <AArch64/AArch64.h>
#include <Library/ArmLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/FdtLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PanicLib.h>
#include <Library/PcdLib.h>
#include <Library/PerformanceLib.h>
#include <Library/SecPlatformSmmuConfigLib.h>

#include <Guid/DxeMemoryProtectionSettings.h>
#include <Guid/FdtHob.h>

// Number of Virtual Memory Map Descriptors
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS  5

#define RESOURCE_CAP  (EFI_RESOURCE_ATTRIBUTE_PRESENT | \
                      EFI_RESOURCE_ATTRIBUTE_INITIALIZED | \
                      EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE | \
                      EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE | \
                      EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | \
                      EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE | \
                      EFI_RESOURCE_ATTRIBUTE_TESTED \
                      )

#define MMIO_CAP  (EFI_RESOURCE_ATTRIBUTE_PRESENT | \
                   EFI_RESOURCE_ATTRIBUTE_INITIALIZED | \
                   EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE | \
                   EFI_RESOURCE_ATTRIBUTE_TESTED \
                   )

//
// Common attribute set used for every DRAM bank we report to DXE.
//
#define DRAM_RESOURCE_ATTRIBUTES  (EFI_RESOURCE_ATTRIBUTE_PRESENT | \
                                  EFI_RESOURCE_ATTRIBUTE_INITIALIZED | \
                                  EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE | \
                                  EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | \
                                  EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE | \
                                  EFI_RESOURCE_ATTRIBUTE_TESTED \
                                  )

/**
  Return the current Boot Mode.

  This function returns the boot reason on the platform

  @return   Return the current Boot Mode of the platform

**/
EFI_BOOT_MODE
ArmPlatformGetBootMode (
  VOID
  )
{
  return BOOT_WITH_FULL_CONFIGURATION;
}

/**
  Initialize controllers that must setup in the normal world.

  This function is called by the ArmPlatformPkg/PrePi or
  ArmPlatformPkg/PlatformPei in the PEI phase.

  @param[in]     MpId               ID of the calling CPU

  @return        RETURN_SUCCESS unless the operation failed
**/
RETURN_STATUS
ArmPlatformInitialize (
  IN  UINTN  MpId
  )
{
  return RETURN_SUCCESS;
}

/**
  Initialize the memory configuration for the platform based on the device tree blob.

  This function will parse the device tree blob to find the memory nodes and create
  the necessary HOBs to describe the system memory layout for the PEI phase.

  @return  EFI_INVALID_PARAMETER  One or more parameters are invalid.
  @return  EFI_SUCCESS            The memory configuration was initialized successfully.
**/
EFI_STATUS
InitializeMemoryConfiguration (
  OUT EFI_PHYSICAL_ADDRESS  *UefiMemoryBase,
  OUT UINT64                *UefiMemorySize
  )
{
  VOID                            *DeviceTreeBase;
  INT32                           Node;
  INT32                           Prev;
  UINT64                          PrimaryBase;
  UINT64                          PrimarySize;
  UINT64                          CurBase;
  UINT64                          CurSize;
  UINT64                          FdtPointer;
  BOOLEAN                         FoundPrimary;
  CONST CHAR8                     *Type;
  INT32                           Len;
  CONST FDT_PROPERTY              *PropertyPtr;
  DXE_MEMORY_PROTECTION_SETTINGS  DxeSettings;
  UINTN                           FdtSize;
  CONST UINT64                    SystemMemoryBase = FixedPcdGet64 (PcdSystemMemoryBase);
  CONST UINT64                    SystemMemorySize = PcdGet64 (PcdSystemMemorySize);
  CONST UINT64                    MmBufferBase     = PcdGet64 (PcdMmBufferBase);
  CONST UINT64                    MmBufferSize     = PcdGet64 (PcdMmBufferSize);
  UINT64                          CurEnd;
  UINT64                          MmBufferEnd;
  UINT64                          OverlapBase;
  UINT64                          OverlapEnd;
  EFI_STATUS                      Status;

  if ((UefiMemoryBase == NULL) || (UefiMemorySize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (FeaturePcdGet (PcdEnableMemoryProtection) == TRUE) {
    DxeSettings = (DXE_MEMORY_PROTECTION_SETTINGS)DXE_MEMORY_PROTECTION_SETTINGS_DEBUG;

    DxeSettings.ImageProtectionPolicy.Fields.ProtectImageFromUnknown = 1;

    // ARM64 does not support having page or pool guards set for these memory types
    DxeSettings.HeapGuardPageType.Fields.EfiACPIMemoryNVS       = 0;
    DxeSettings.HeapGuardPageType.Fields.EfiReservedMemoryType  = 0;
    DxeSettings.HeapGuardPageType.Fields.EfiRuntimeServicesCode = 0;
    DxeSettings.HeapGuardPageType.Fields.EfiRuntimeServicesData = 0;
    DxeSettings.HeapGuardPoolType.Fields.EfiACPIMemoryNVS       = 0;
    DxeSettings.HeapGuardPoolType.Fields.EfiReservedMemoryType  = 0;
    DxeSettings.HeapGuardPoolType.Fields.EfiRuntimeServicesCode = 0;
    DxeSettings.HeapGuardPoolType.Fields.EfiRuntimeServicesData = 0;

    // THE /NXCOMPAT DLL flag is not set on grub/shim today, so do not block loading
    // otherwise we cannot boot Linux
    DxeSettings.ImageProtectionPolicy.Fields.BlockImagesWithoutNxFlag = 0;

    // Patina does not currently support page/pool guard, so disable them to avoid shell tests from expecting them
    DxeSettings.HeapGuardPolicy.Fields.UefiPageGuard = 0;
    DxeSettings.HeapGuardPolicy.Fields.UefiPoolGuard = 0;

    BuildGuidDataHob (
      &gDxeMemoryProtectionSettingsGuid,
      &DxeSettings,
      sizeof (DxeSettings)
      );
  }

  DeviceTreeBase = (VOID *)(UINTN)PcdGet64 (PcdDeviceTreeInitialBaseAddress);
  if (DeviceTreeBase == NULL) {
    PANIC ("Device Tree Base Address is not set. Cannot continue without a valid Device Tree Blob.\n");
  }

  // Make sure we have a valid device tree blob
  if (FdtCheckHeader (DeviceTreeBase) != 0) {
    PANIC ("Device Tree Blob header is not valid. Cannot continue without a valid Device Tree Blob.\n");
  }

  // Walk every memory@... node in the FDT and emit one resource descriptor
  // HOB per bank, instead of remembering only the lowest. The bank whose base
  // matches PcdSystemMemoryBase is recorded as the PEI memory window that
  // gets handed back to PrePi.
  PrimaryBase  = 0;
  PrimarySize  = 0;
  FoundPrimary = FALSE;
  MmBufferEnd  = MmBufferBase + MmBufferSize;

  if ((MmBufferSize != 0) && (MmBufferEnd < MmBufferBase)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: MmBuffer range overflows (Base=0x%lx Size=0x%lx)\n",
      __func__,
      MmBufferBase,
      MmBufferSize
      ));
    return EFI_DEVICE_ERROR;
  }

  for (Prev = 0; ; Prev = Node) {
    Node = FdtNextNode (DeviceTreeBase, Prev, NULL);
    if (Node < 0) {
      break;
    }

    PropertyPtr = FdtGetProperty (DeviceTreeBase, Node, "device_type", &Len);
    if ((PropertyPtr == NULL) || (PropertyPtr->Data == NULL)) {
      continue;
    }

    Type = PropertyPtr->Data;
    DEBUG ((
      DEBUG_INFO,
      "%a: Found device tree node with type %.*a\n",
      __func__,
      Len,
      Type
      ));

    if (AsciiStrnCmp (Type, "memory", Len) != 0) {
      continue;
    }

    //
    // Nodes with "okay" or "ok" are secure memory, skip them.
    //
    PropertyPtr = FdtGetProperty (DeviceTreeBase, Node, "status", &Len);
    if ((PropertyPtr != NULL) && (PropertyPtr->Data != NULL) && (Len > 0)) {
      if ((AsciiStrnCmp ((CONST CHAR8 *)PropertyPtr->Data, "okay", Len) != 0) &&
          (AsciiStrnCmp ((CONST CHAR8 *)PropertyPtr->Data, "ok", Len) != 0))
      {
        DEBUG ((
          DEBUG_INFO,
          "%a: Skipping memory node (status=\"%.*a\")\n",
          __func__,
          Len,
          (CONST CHAR8 *)PropertyPtr->Data
          ));
        continue;
      }
    }

    // Get the 'reg' property of this node. We assume two 8-byte quantities
    // for base and size, respectively.
    PropertyPtr = FdtGetProperty (DeviceTreeBase, Node, "reg", &Len);
    if (PropertyPtr == NULL) {
      DEBUG ((DEBUG_ERROR, "%a: Memory node has no 'reg' property\n", __func__));
      continue;
    }

    if (Len < (INT32)(2 * sizeof (UINT64))) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Unexpected 'reg' length %d on memory node\n",
        __func__,
        Len
        ));
      continue;
    }

    CurBase = Fdt64ToCpu (ReadUnaligned64 ((UINT64 *)PropertyPtr->Data));
    CurSize = Fdt64ToCpu (ReadUnaligned64 ((UINT64 *)(PropertyPtr->Data) + 1));

    DEBUG ((
      DEBUG_INFO,
      "%a: System RAM @ 0x%lx - 0x%lx\n",
      __func__,
      CurBase,
      CurBase + CurSize - 1
      ));

    CurEnd = CurBase + CurSize;
    if (CurEnd < CurBase) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Memory node range overflows (Base=0x%lx Size=0x%lx)\n",
        __func__,
        CurBase,
        CurSize
        ));
      continue;
    }

    // Carve the MM buffer hole out of the system-memory resource map.
    if ((MmBufferSize != 0) && (CurBase < MmBufferEnd) && (MmBufferBase < CurEnd)) {
      OverlapBase = (CurBase > MmBufferBase) ? CurBase : MmBufferBase;
      OverlapEnd  = (CurEnd  < MmBufferEnd) ? CurEnd  : MmBufferEnd;

      if (OverlapBase > CurBase) {
        BuildResourceDescriptorV2 (
          EFI_RESOURCE_SYSTEM_MEMORY,
          DRAM_RESOURCE_ATTRIBUTES,
          CurBase,
          OverlapBase - CurBase,
          EFI_MEMORY_WB,
          NULL
          );
      }

      if (OverlapEnd < CurEnd) {
        BuildResourceDescriptorV2 (
          EFI_RESOURCE_SYSTEM_MEMORY,
          DRAM_RESOURCE_ATTRIBUTES,
          OverlapEnd,
          CurEnd - OverlapEnd,
          EFI_MEMORY_WB,
          NULL
          );
      }
    } else {
      BuildResourceDescriptorV2 (
        EFI_RESOURCE_SYSTEM_MEMORY,
        DRAM_RESOURCE_ATTRIBUTES,
        CurBase,
        CurSize,
        EFI_MEMORY_WB,
        NULL
        );
    }

    if (CurBase == SystemMemoryBase) {
      PrimaryBase  = CurBase;
      PrimarySize  = CurSize;
      FoundPrimary = TRUE;
    }
  }

  FdtSize = FdtTotalSize (DeviceTreeBase) + PcdGet32 (PcdDeviceTreeAllocationPadding);

  // Publish the FDT base address as a gFdtHobGuid HOB so that DXE consumers
  // (e.g. FdtClientDxe and FdtPciHostBridgeLib) can locate the device tree.
  // FdtClientDxe expects exactly sizeof(UINT64) of payload containing the
  // DTB base address.
  FdtPointer = (UINT64)(UINTN)DeviceTreeBase;
  BuildGuidDataHob (&gFdtHobGuid, &FdtPointer, sizeof (FdtPointer));

  // Create a memory allocation HOB for the device tree blob
  BuildMemoryAllocationHob (
    (EFI_PHYSICAL_ADDRESS)(((UINTN)DeviceTreeBase / EFI_PAGE_SIZE) * EFI_PAGE_SIZE),
    ALIGN_VALUE (FdtSize, EFI_PAGE_SIZE),
    EfiBootServicesData
    );

  // Make sure the FDT actually advertises the bank we expect to host PEI.
  if (!FoundPrimary) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: No FDT memory node matches PcdSystemMemoryBase = 0x%lx\n",
      __func__,
      SystemMemoryBase
      ));
    PANIC ("System Memory Base Mismatch.\n");
    return EFI_DEVICE_ERROR;
  }

  // Preserve legacy PrePi heap-window semantics: if the primary bank is at
  // least as large as the configured PEI window, hand PrePi the entire bank;
  // otherwise hand it PcdSystemMemorySize. This mirrors the prior behavior.
  if (PrimarySize < SystemMemorySize) {
    PrimarySize = SystemMemorySize;
  }

  Status = BuildSmmuConfigHob ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to build SMMU Config HOB\n", __func__));
    return Status;
  }

  *UefiMemoryBase = PrimaryBase;
  *UefiMemorySize = PrimarySize;

  return EFI_SUCCESS;
}

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU
  on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR
                                    describing a Physical-to-Virtual Memory
                                    mapping. This array must be ended by a
                                    zero-filled entry. The allocated memory
                                    will not be freed.

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  OUT ARM_MEMORY_REGION_DESCRIPTOR  **VirtualMemoryMap
  )
{
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;
  UINT64                        TpmBase;
  UINT32                        TpmSize;
  EFI_PHYSICAL_ADDRESS          UefiMemoryBase;
  UINT64                        UefiMemorySize;
  EFI_STATUS                    Status;

  TpmBase = PcdGet64 (PcdTpmBaseAddress);
  TpmSize = PcdGet32 (PcdTpmCrbRegionSize);

  if (TpmBase != 0) {
    DEBUG ((DEBUG_INFO, "%a: TPM @ 0x%lx\n", __func__, TpmBase));
    BuildMemoryAllocationHob (TpmBase, TpmSize, EfiACPIMemoryNVS);
  }

  BuildMemoryAllocationHob (
    PcdGet64 (PcdAdvancedLoggerBase),
    PcdGet32 (PcdAdvancedLoggerPages) * EFI_PAGE_SIZE,
    EfiRuntimeServicesData
    );

  VirtualMemoryTable = AllocatePool (
                         sizeof (ARM_MEMORY_REGION_DESCRIPTOR) *
                         MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS
                         );

  if (VirtualMemoryTable == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Error: Failed AllocatePool()\n", __func__));
    return;
  }

  Status = InitializeMemoryConfiguration (&UefiMemoryBase, &UefiMemorySize);
  if (EFI_ERROR (Status)) {
    PANIC ("Failed to initialize memory configuration from device tree.\n");
    return;
  }

  // System DRAM
  VirtualMemoryTable[0].PhysicalBase = UefiMemoryBase;
  VirtualMemoryTable[0].VirtualBase  = VirtualMemoryTable[0].PhysicalBase;
  VirtualMemoryTable[0].Length       = UefiMemorySize;
  VirtualMemoryTable[0].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  DEBUG ((
    DEBUG_INFO,
    "%a: Dumping System DRAM Memory Map:\n"
    "\tPhysicalBase: 0x%lX\n"
    "\tVirtualBase: 0x%lX\n"
    "\tLength: 0x%lX\n",
    __func__,
    VirtualMemoryTable[0].PhysicalBase,
    VirtualMemoryTable[0].VirtualBase,
    VirtualMemoryTable[0].Length
    ));

  // Peripheral space before DRAM
  VirtualMemoryTable[1].PhysicalBase = 0x0;
  VirtualMemoryTable[1].VirtualBase  = 0x0;
  VirtualMemoryTable[1].Length       = VirtualMemoryTable[0].PhysicalBase;
  VirtualMemoryTable[1].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // Secure Flash, do not touch
  BuildResourceDescriptorV2 (
    EFI_RESOURCE_FIRMWARE_DEVICE,
    RESOURCE_CAP,
    0,
    0x04000000,
    EFI_MEMORY_UC,
    NULL
    );

  // Flash
  BuildResourceDescriptorV2 (
    EFI_RESOURCE_FIRMWARE_DEVICE,
    RESOURCE_CAP,
    0x04000000,
    0x04000000,
    EFI_MEMORY_WB,
    NULL
    );

  // GIC_D
  BuildResourceDescriptorV2 (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    MMIO_CAP,
    0x08000000,
    0x00010000,
    EFI_MEMORY_UC,
    NULL
    );

  // GIC_R
  BuildResourceDescriptorV2 (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    MMIO_CAP,
    0x080A0000,
    0x00F60000,
    EFI_MEMORY_UC,
    NULL
    );

  // UART
  BuildResourceDescriptorV2 (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    MMIO_CAP,
    0x09000000,
    0x1000,
    EFI_MEMORY_UC,
    NULL
    );

  // FW_CFG: This is not even 1 page long in QEMU, but we need to reserve the entire
  // region to make Patina happy.
  BuildResourceDescriptorV2 (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    MMIO_CAP,
    0x09020000,
    0x00001000,
    EFI_MEMORY_UC,
    NULL
    );

  // SMMU
  BuildResourceDescriptorV2 (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    MMIO_CAP,
    0x09050000,
    0x00020000,
    EFI_MEMORY_UC,
    NULL
    );

  // PCIE_PIO will be handled in FdtPciHostBridgeLib

  // PCIE_MMIO
  BuildResourceDescriptorV2 (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    MMIO_CAP,
    0x10000000,
    0x2eff0000,
    EFI_MEMORY_UC,
    NULL
    );

  // PCIE_ECAM
  BuildResourceDescriptorV2 (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    MMIO_CAP,
    0x3f000000,
    0x01000000,
    EFI_MEMORY_UC,
    NULL
    );

  // Remap the FD region as normal executable memory
  VirtualMemoryTable[2].PhysicalBase = PcdGet64 (PcdFdBaseAddress);
  VirtualMemoryTable[2].VirtualBase  = VirtualMemoryTable[2].PhysicalBase;
  VirtualMemoryTable[2].Length       = FixedPcdGet32 (PcdFdSize);
  VirtualMemoryTable[2].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // MM Memory Space
  VirtualMemoryTable[3].PhysicalBase = PcdGet64 (PcdMmBufferBase);
  VirtualMemoryTable[3].VirtualBase  = PcdGet64 (PcdMmBufferBase);
  VirtualMemoryTable[3].Length       = PcdGet64 (PcdMmBufferSize);
  VirtualMemoryTable[3].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  BuildResourceDescriptorV2 (
    EFI_RESOURCE_MEMORY_RESERVED,
    RESOURCE_CAP,
    VirtualMemoryTable[3].PhysicalBase,
    VirtualMemoryTable[3].Length,
    EFI_MEMORY_WB,
    NULL
    );

  // End of Table
  ZeroMem (&VirtualMemoryTable[4], sizeof (ARM_MEMORY_REGION_DESCRIPTOR));

  *VirtualMemoryMap = VirtualMemoryTable;
}
