/** @file
  Mock platform verified boot PEIM for ECIT capability HOB reporting.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiPei.h>
#include <Library/DebugLib.h>
#include <Library/EcitReportLib.h>
#include <Library/PeimEntryPoint.h>

STATIC CONST EFI_GUID  mMockPlatformVerifiedBootFeatureGuid = {
  0xf2603026, 0x19d4, 0x4e07, { 0x8f, 0xd5, 0x14, 0xbc, 0xc3, 0x05, 0x8a, 0xfd }
};

STATIC CONST CHAR8  mMockPlatformVerifiedBootPayload[] = "1.2.3";

/**
  Report one test-only ECIT capability record from PEI.

  @param[in] FileHandle  Handle of the file being invoked.
  @param[in] PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCCESS  The test capability record was reported.
  @retval Others       The capability record could not be reported.
**/
EFI_STATUS
EFIAPI
MockPlatformVerifiedBootPeiEntryPoint (
  IN EFI_PEI_FILE_HANDLE        FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS  Status;

  Status = EcitReportCapability (
             &mMockPlatformVerifiedBootFeatureGuid,
             mMockPlatformVerifiedBootPayload,
             sizeof (mMockPlatformVerifiedBootPayload)
             );
  DEBUG ((
    EFI_ERROR (Status) ? DEBUG_ERROR : DEBUG_INFO,
    "MockPlatformVerifiedBootPei: report capability - %r\n",
    Status
    ));

  return Status;
}
