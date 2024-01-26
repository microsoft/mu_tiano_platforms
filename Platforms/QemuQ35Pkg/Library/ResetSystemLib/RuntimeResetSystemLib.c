/** @file
  Reset System Library Shutdown API implementation that can be used at
  runtime.

  Copyright (C) 2020, Red Hat, Inc.
  Copyright (c) 2006 - 2019, Intel Corporation. All rights reserved.<BR>
  Copyright (c) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>                   // BIT13

#include <Library/BaseLib.h>        // CpuDeadLoop()
#include <Library/DebugLib.h>       // ASSERT()
#include <Library/IoLib.h>          // IoOr16()
#include <Library/PcdLib.h>         // PcdGet16()
#include <Library/ResetSystemLib.h> // ResetShutdown()
#include <OvmfPlatforms.h>          // PIIX4_PMBA_VALUE

#include "ResetSystemLibInternal.h"

STATIC UINT16  mAcpiPmBaseAddress;
STATIC UINT16  mAcpiHwReducedSleepCtl;

/**
  Stores hardware reset context for later use.

  @retval EFI_SUCCESS     The hardware information was saved successfully.
  @retval EFI_UNSUPPORTED The host bridge PCI device ID is invalid or unexpected.

**/
EFI_STATUS
SaveHardwareContext (
  VOID
  )
{
  UINT16  HostBridgeDevId;

  HostBridgeDevId = PcdGet16 (PcdOvmfHostBridgePciDevId);
  switch (HostBridgeDevId) {
    case INTEL_82441_DEVICE_ID:
      mAcpiPmBaseAddress = PIIX4_PMBA_VALUE;
      break;
    case INTEL_Q35_MCH_DEVICE_ID:
      mAcpiPmBaseAddress = ICH9_PMBASE_VALUE;
      break;
    case CLOUDHV_DEVICE_ID:
      mAcpiHwReducedSleepCtl = CLOUDHV_ACPI_SHUTDOWN_IO_ADDRESS;
      break;
    default:
      ASSERT (FALSE);
      CpuDeadLoop ();
      return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Calling this function causes the system to enter a power state equivalent
  to the ACPI G2/S5 or G3 states.

  System shutdown should not return, if it returns, it means the system does
  not support shut down reset.
**/
VOID
EFIAPI
ResetShutdown (
  VOID
  )
{
  if (mAcpiHwReducedSleepCtl) {
    IoWrite8 (mAcpiHwReducedSleepCtl, 5 << 2 | 1 << 5);
  } else {
    IoBitFieldWrite16 (mAcpiPmBaseAddress + 4, 10, 13, 0);
    IoOr16 (mAcpiPmBaseAddress + 4, BIT13);
  }

  CpuDeadLoop ();
}
