/** @file

  Copyright (c) 2011-2014, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include <Library/ArmPlatformLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PanicLib.h>
#include <libfdt.h>
#include <Library/HobLib.h>
#include <Guid/DxeMemoryProtectionSettings.h>
#include <Guid/MmMemoryProtectionSettings.h>

#include <libfdt.h>

STATIC CONST EFI_PEI_PPI_DESCRIPTOR  mTpm2DiscoveredPpi = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gQemuPkgTpmDiscoveredPpiGuid,
  NULL
};

STATIC CONST EFI_PEI_PPI_DESCRIPTOR  mTpm2InitializationDonePpi = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gPeiTpmInitializationDonePpiGuid,
  NULL
};

VOID
InitializeMemory (
  VOID
)
{
  VOID                            *DeviceTreeBase;
  INT32                           Node, Prev;
  UINT64                          NewBase, CurBase;
  UINT64                          NewSize, CurSize;
  CONST CHAR8                     *Type;
  INT32                           Len;
  CONST UINT64                    *RegProp;
  RETURN_STATUS                   PcdStatus;
  DXE_MEMORY_PROTECTION_SETTINGS  DxeSettings;
  MM_MEMORY_PROTECTION_SETTINGS   MmSettings;
  UINTN                           FdtSize;

  if (FeaturePcdGet (PcdEnableMemoryProtection) == TRUE) {
    DxeSettings = (DXE_MEMORY_PROTECTION_SETTINGS)DXE_MEMORY_PROTECTION_SETTINGS_DEBUG;
    MmSettings  = (MM_MEMORY_PROTECTION_SETTINGS)MM_MEMORY_PROTECTION_SETTINGS_DEBUG;

    MmSettings.HeapGuardPolicy.Fields.MmPageGuard                    = 1;
    MmSettings.HeapGuardPolicy.Fields.MmPoolGuard                    = 1;
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

    BuildGuidDataHob (
      &gMmMemoryProtectionSettingsGuid,
      &MmSettings,
      sizeof (MmSettings)
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
    PANIC ("Device Tree Blob at 0x%p is not valid. Cannot continue without a valid Device Tree Blob.\n", DeviceTreeBase);
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
    PANIC (
      "System Memory Base Mismatch. Expected 0x%lx, but found 0x%lx.\n",
      FixedPcdGet64 (PcdSystemMemoryBase),
      NewBase
      );
  }

  PcdStatus = PcdSet64S (PcdSystemMemorySize, NewSize - PcdGet64 (PcdMmBufferSize));
  if (EFI_ERROR (PcdStatus)) {
    PANIC (
      "Failed to set PcdSystemMemorySize to 0x%lx. Status = %r\n",
      NewSize - PcdGet64 (PcdMmBufferSize),
      PcdStatus
      );
  }

  PcdStatus = PcdSet64S (PcdMmBufferBase, CurBase + NewSize - PcdGet64 (PcdMmBufferSize));
  if (EFI_ERROR (PcdStatus)) {
    PANIC (
      "Failed to set PcdMmBufferBase to 0x%lx. Status = %r\n",
      CurBase + NewSize - PcdGet64 (PcdMmBufferSize),
      PcdStatus
      );
  }
}

EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  )
{
  UINT64      TpmBase;
  UINT32      TpmSize;
  EFI_STATUS  Status;

  TpmBase = PcdGet64 (PcdTpmBaseAddress);
  TpmSize = PcdGet32 (PcdTpmCrbRegionSize);

  if (TpmBase != 0) {
    DEBUG ((DEBUG_INFO, "%a: TPM @ 0x%lx\n", __func__, TpmBase));

    Status = PeiServicesInstallPpi (&mTpm2DiscoveredPpi);
    BuildMemoryAllocationHob (TpmBase, TpmSize, EfiACPIMemoryNVS);
  } else {
    Status = PeiServicesInstallPpi (&mTpm2InitializationDonePpi);
  }

  if (EFI_ERROR (Status)) {
    PANIC ("%a: Failed to install TPM PPI. Status = %r\n", __func__, Status);
  }

  BuildFvHob (PcdGet64 (PcdFvBaseAddress), PcdGet32 (PcdFvSize));

  InitializeMemory ();

  return EFI_SUCCESS;
}
