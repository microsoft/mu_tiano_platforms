/** @file
  Consumer module to locate conf data from variable storage, initialize
  the GFX policy data and override the policy based on configuration values.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <PolicyDataStructGFX.h>

#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/PolicyLib.h>
#include <Library/MemoryAllocationLib.h>

// XML autogen definitions
#include <Generated/ConfigClientGenerated.h>

// Statically define policy initialization for 2 GFX ports
GFX_POLICY_DATA  DefaultQemuGfxPolicy[GFX_PORT_MAX_CNT] = {
  {
    .Power_State_Port = TRUE
  },
  {
    .Power_State_Port = TRUE
  }
};

/**
  Helper function to apply GFX configuration data to GFX silicon policy.

  @param[in]      GfxConfigBuffer   Pointer to GFX configuration data.

  @retval EFI_SUCCESS           The configuration is translated to policy successfully.
  @retval EFI_INVALID_PARAMETER One or more of the required input pointers are NULL.
  @retval EFI_BUFFER_TOO_SMALL  Supplied GfxSiliconPolicy is too small to fit in translated data.
  @retval Others                Other errors occurred when getting GFX policy.
**/
EFI_STATUS
EFIAPI
ApplyGfxConfigToPolicy (
  IN  VOID  *ConfigBuffer
  )
{
  EFI_STATUS       Status;
  BOOLEAN          GfxEnablePort0;
  GFX_POLICY_DATA  *GfxSiliconPolicy;

  if (ConfigBuffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((DEBUG_INFO, "%a Entry...\n", __func__));

  // query autogen header to get config knob value
  GfxEnablePort0   = *(BOOLEAN *)ConfigBuffer;
  GfxSiliconPolicy = AllocateCopyPool (sizeof (DefaultQemuGfxPolicy), DefaultQemuGfxPolicy);

  // We only translate the GFX ports #0 exposed to platform from conf data
  GfxSiliconPolicy[0].Power_State_Port = GfxEnablePort0;

  Status = SetPolicy (&gSbsaPolicyDataGFXGuid, POLICY_ATTRIBUTE_FINALIZED, GfxSiliconPolicy, sizeof (DefaultQemuGfxPolicy));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to update GFX policy per configuration data - %r!!!\n", __func__, Status));
    ASSERT (FALSE);
    goto Exit;
  }

Exit:
  DEBUG ((DEBUG_INFO, "%a Exit - %r\n", __func__, Status));
  return Status;
}
