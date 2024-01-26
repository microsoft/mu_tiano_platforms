/** @file
  TPM Replay PEI FV Exclusion - Main File

  Contains logic to exclude FVs for this platform so measurements can be made
  on a clean slate.

  Copyright (c) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Ppi/FirmwareVolumeInfoMeasurementExcluded.h>

EFI_PEI_FIRMWARE_VOLUME_INFO_MEASUREMENT_EXCLUDED_FV  mFvMeasurementExclusionsTable[] = {
  { (EFI_PHYSICAL_ADDRESS)FixedPcdGet32 (PcdOvmfPeiMemFvBase), (UINT64)FixedPcdGet32 (PcdOvmfPeiMemFvSize) },
  { (EFI_PHYSICAL_ADDRESS)FixedPcdGet32 (PcdOvmfDxeMemFvBase), (UINT64)FixedPcdGet32 (PcdOvmfDxeMemFvSize) }
};

EFI_STATUS
EFIAPI
GetPlatformFvExclusions (
  OUT CONST EFI_PEI_FIRMWARE_VOLUME_INFO_MEASUREMENT_EXCLUDED_FV  **ExcludedFvs,
  OUT UINTN                                                       *ExcludedFvsCount
  )
{
  *ExcludedFvs      = &mFvMeasurementExclusionsTable[0];
  *ExcludedFvsCount = ARRAY_SIZE (mFvMeasurementExclusionsTable);

  return EFI_SUCCESS;
}
