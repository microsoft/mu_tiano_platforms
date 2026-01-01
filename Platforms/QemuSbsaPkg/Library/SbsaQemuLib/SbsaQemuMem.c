/** @file

  Copyright (c) 2019, Linaro Limited. All rights reserved.
  Copyright (c) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Uefi.h>
#include <Library/ArmLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PanicLib.h>
#include <libfdt.h>

#include <Guid/DxeMemoryProtectionSettings.h>

// Number of Virtual Memory Map Descriptors
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS  5

// MU_CHANGE START

/**
  Checks if the platform requires a special initial EFI memory region.

  @param[out]  EfiMemoryBase  The custom memory base, will be unchanged if FALSE is returned.
  @param[out]  EfiMemorySize  The custom memory size, will be unchanged if FALSE is returned.

  @retval   TRUE    A custom memory region was set.
  @retval   FALSE   A custom memory region was not set.
**/
BOOLEAN
EFIAPI
ArmPlatformGetPeiMemory (
  OUT UINTN   *EfiMemoryBase,
  OUT UINT32  *EfiMemorySize
  )
{
  return FALSE;
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
  INT32                           Node, Prev;
  UINT64                          NewBase, CurBase;
  UINT64                          NewSize, CurSize;
  CONST CHAR8                     *Type;
  INT32                           Len;
  CONST UINT64                    *RegProp;
  DXE_MEMORY_PROTECTION_SETTINGS  DxeSettings;
  UINTN                           FdtSize;

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

    BuildGuidDataHob (
      &gDxeMemoryProtectionSettingsGuid,
      &DxeSettings,
      sizeof (DxeSettings)
      );
  }
  NewBase = 0;
  NewSize = 0;

  DeviceTreeBase = (VOID *)(UINTN)PcdGet64 (PcdDeviceTreeInitialBaseAddress);
  if (DeviceTreeBase == NULL) {
    PANIC ("Device Tree Base Address is not set. Cannot continue without a valid Device Tree Blob.\n");
  }
  // Make sure we have a valid device tree blob
  if (fdt_check_header (DeviceTreeBase) != 0) {
    PANIC ("Device Tree Blob header is not valid. Cannot continue without a valid Device Tree Blob.\n");
  }
  // Look for the lowest memory node
  for (Prev = 0; ; Prev = Node) {
    Node = fdt_next_node (DeviceTreeBase, Prev, NULL);
    if (Node < 0) {
      break;
    }

    // Check for memory node
    Type = fdt_getprop (DeviceTreeBase, Node, "device_type", &Len);
    if (Type && (AsciiStrnCmp (Type, "memory", Len) == 0)) {
      // Get the 'reg' property of this node. For now, we will assume
      // two 8 byte quantities for base and size, respectively.
      RegProp = fdt_getprop (DeviceTreeBase, Node, "reg", &Len);
      if ((RegProp != 0) && (Len == (2 * sizeof (UINT64)))) {
        CurBase = fdt64_to_cpu (ReadUnaligned64 (RegProp));
        CurSize = fdt64_to_cpu (ReadUnaligned64 (RegProp + 1));

        DEBUG ((
          DEBUG_INFO,
          "%a: System RAM @ 0x%lx - 0x%lx\n",
          __FUNCTION__,
          CurBase,
          CurBase + CurSize - 1
          ));

        if ((NewBase > CurBase) || (NewBase == 0)) {
          NewBase = CurBase;
          NewSize = CurSize;
        }
      } else {
        DEBUG ((
          DEBUG_ERROR,
          "%a: Failed to parse FDT memory node\n",
          __FUNCTION__
          ));
      }
    }
  }

  FdtSize = fdt_totalsize (DeviceTreeBase) + PcdGet32 (PcdDeviceTreeAllocationPadding);

  // Create a memory allocation HOB for the device tree blob
  BuildMemoryAllocationHob (
    (EFI_PHYSICAL_ADDRESS)(((UINTN)DeviceTreeBase / EFI_PAGE_SIZE) * EFI_PAGE_SIZE),
    ALIGN_VALUE (FdtSize, EFI_PAGE_SIZE),
    EfiBootServicesData
    );

    // Make sure the start of DRAM matches our expectation
  if (FixedPcdGet64 (PcdSystemMemoryBase) != NewBase) {
    PANIC ("System Memory Base Mismatch.\n");
    return EFI_DEVICE_ERROR;
  }

  if (NewSize > PcdGet64 (PcdSystemMemorySize)) {
    BuildResourceDescriptorV2 (
      EFI_RESOURCE_SYSTEM_MEMORY,
      (EFI_RESOURCE_ATTRIBUTE_PRESENT |
       EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
       EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
       EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
       EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
       EFI_RESOURCE_ATTRIBUTE_TESTED),
      NewBase + PcdGet64 (PcdSystemMemorySize),
      NewSize - PcdGet64 (PcdSystemMemorySize),
      EFI_MEMORY_WB,
      NULL
      );
  } else {
    NewSize = PcdGet64 (PcdSystemMemorySize);
  }

  BuildResourceDescriptorV2 (
    EFI_RESOURCE_SYSTEM_MEMORY,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
     EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
     EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
     EFI_RESOURCE_ATTRIBUTE_TESTED),
    NewBase,
    PcdGet64 (PcdSystemMemorySize),
    EFI_MEMORY_WB,
    NULL
    );

  *UefiMemoryBase = NewBase;
  *UefiMemorySize = NewSize;

  return EFI_SUCCESS;
}

// MU_CHANGE END

RETURN_STATUS
EFIAPI
SbsaQemuLibConstructor (
  VOID
  )
{
  return RETURN_SUCCESS;
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

  UINT64      TpmBase;
  UINT32      TpmSize;

  EFI_PHYSICAL_ADDRESS  UefiMemoryBase;
  UINT64                UefiMemorySize;
  EFI_STATUS           Status;

  TpmBase = PcdGet64 (PcdTpmBaseAddress);
  TpmSize = PcdGet32 (PcdTpmCrbRegionSize);

  if (TpmBase != 0) {
    DEBUG ((DEBUG_INFO, "%a: TPM @ 0x%lx\n", __func__, TpmBase));
    BuildMemoryAllocationHob (TpmBase, TpmSize, EfiACPIMemoryNVS);
  }

  BuildMemoryAllocationHob (
    PcdGet64 (PcdMmBufferBase),
    PcdGet64 (PcdMmBufferSize),
    EfiReservedMemoryType
    );

  BuildMemoryAllocationHob (
    PcdGet64 (PcdAdvancedLoggerBase),
    PcdGet32 (PcdAdvancedLoggerPages) * EFI_PAGE_SIZE,
    EfiRuntimeServicesData
    );

  ASSERT (VirtualMemoryMap != NULL);

  VirtualMemoryTable = AllocatePool (
                         sizeof (ARM_MEMORY_REGION_DESCRIPTOR) *
                         MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS
                         );

  if (VirtualMemoryTable == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Error: Failed AllocatePool()\n", __FUNCTION__));
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
    __FUNCTION__,
    VirtualMemoryTable[0].PhysicalBase,
    VirtualMemoryTable[0].VirtualBase,
    VirtualMemoryTable[0].Length
    ));

  // Peripheral space before DRAM
  VirtualMemoryTable[1].PhysicalBase = 0x0;
  VirtualMemoryTable[1].VirtualBase  = 0x0;
  VirtualMemoryTable[1].Length       = VirtualMemoryTable[0].PhysicalBase;
  VirtualMemoryTable[1].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // Remap the FD region as normal executable memory
  VirtualMemoryTable[2].PhysicalBase = PcdGet64 (PcdFdBaseAddress);
  VirtualMemoryTable[2].VirtualBase  = VirtualMemoryTable[2].PhysicalBase;
  VirtualMemoryTable[2].Length       = FixedPcdGet32 (PcdFdSize);
  VirtualMemoryTable[2].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // MM Memory Space
  VirtualMemoryTable[3].PhysicalBase = PcdGet64 (PcdMmBufferBase);
  VirtualMemoryTable[3].VirtualBase  = PcdGet64 (PcdMmBufferBase);
  VirtualMemoryTable[3].Length       = PcdGet64 (PcdMmBufferSize);
  VirtualMemoryTable[3].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // End of Table
  ZeroMem (&VirtualMemoryTable[4], sizeof (ARM_MEMORY_REGION_DESCRIPTOR));

  *VirtualMemoryMap = VirtualMemoryTable;
}
