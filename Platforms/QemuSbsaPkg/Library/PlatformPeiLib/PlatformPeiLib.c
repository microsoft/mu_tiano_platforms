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

EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  )
{
  UINT64      TpmBase;
  EFI_STATUS  Status;

  TpmBase = PcdGet64 (PcdTpmBaseAddress);

  if (TpmBase != 0) {
    DEBUG ((DEBUG_INFO, "%a: TPM @ 0x%lx\n", __func__, TpmBase));

    Status = PeiServicesInstallPpi (&mTpm2DiscoveredPpi);
  } else {
    Status = PeiServicesInstallPpi (&mTpm2InitializationDonePpi);
  }

  ASSERT_EFI_ERROR (Status);

  BuildFvHob (PcdGet64 (PcdFvBaseAddress), PcdGet32 (PcdFvSize));

  return EFI_SUCCESS;
}
