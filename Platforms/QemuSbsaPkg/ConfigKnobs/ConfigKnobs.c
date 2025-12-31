/** @file
  Consumer module to locate conf data from variable storage, initialize
  the GFX policy data and override the policy based on configuration values.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <PolicyDataStructGFX.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PolicyLib.h>
#include <ConfigStdStructDefs.h>

// XML autogen definitions
#include <Generated/ConfigClientGenerated.h>
#include <Generated/ConfigServiceGenerated.h>

#include "ConfigKnobs.h"

/**
  Module entry point that will check configuration data and publish them to policy database.

  @param    ImageHandle     Image handle of this driver.
  @param    SystemTable     Pointer to the system table.

  @retval Status                        From internal routine or boot object, should not fail
**/
EFI_STATUS
EFIAPI
ConfigKnobsEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  BOOLEAN  GfxEnablePort0;

  DEBUG ((DEBUG_INFO, "%a - Entry.\n", __FUNCTION__));

  Status = ConfigGetPowerOnPort0 (&GfxEnablePort0);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to get PowerOnPort0 config knob! - %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Done;
  }

  Status = ApplyGfxConfigToPolicy (&GfxEnablePort0);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to apply configuration data to the GFX silicon policy - %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Done;
  }

Done:

  return Status;
}
