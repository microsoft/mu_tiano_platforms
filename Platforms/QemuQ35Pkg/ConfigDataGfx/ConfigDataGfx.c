/** @file
  Consumer module to locate conf data from variable storage, initialize
  the GFX policy data and override the policy based on configuration values.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <PolicyDataStructGFX.h>
#include <Protocol/Policy.h>

#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/Policy.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/ConfigDataLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

#include <ConfigDataStruct.h>       // For definitions derived from yaml files

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
  Helper function to translate GFX configuration data to GFX silicon policy.

  @param[in]      PolicyInterface   Pointer to current policy protocol/PPI interface.
  @param[in]      PlatformGfxCfg    Pointer to GFX configuration data.
  @param[in]      ConfSize          The size of PlatformGfxCfg.
  @param[out]     GfxSiliconPolicy  Optional pointer to hold translated GFX silicon policy.
                                    May be NULL with a zero PolicySize in order to determine
                                    the size buffer needed.
  @param[in, out] PolicySize        On input, this is the available size of GfxSiliconPolicy,
                                    on output, this is the size of translated GfxSiliconPolicy.

  @retval EFI_SUCCESS           The configuration is translated to policy successfully.
  @retval EFI_INVALID_PARAMETER One or more of the required input pointers are NULL.
  @retval EFI_BUFFER_TOO_SMALL  Supplied GfxSiliconPolicy is too small to fit in translated data.
  @retval Others                Other errors occurred when getting GFX policy.
**/
STATIC
EFI_STATUS
ConvertGfxPolicyFromConfData (
  IN      POLICY_PROTOCOL  *PolicyInterface,
  IN      GFX_CFG_DATA     *PlatformGfxCfg,
  IN      UINTN            ConfSize,
  OUT     GFX_POLICY_DATA  *GfxSiliconPolicy OPTIONAL,
  IN OUT  UINT16           *PolicySize
  )
{
  EFI_STATUS  Status;

  if ((PolicyInterface == NULL) ||
      (GfxSiliconPolicy == NULL) || (PolicySize == NULL) ||
      (PlatformGfxCfg == NULL) || (ConfSize == 0))
  {
    return EFI_INVALID_PARAMETER;
  }

  if (*PolicySize < GFX_POLICY_SIZE) {
    return EFI_BUFFER_TOO_SMALL;
  }

  Status = PolicyInterface->GetPolicy (&gPolicyDataGFXGuid, NULL, GfxSiliconPolicy, PolicySize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // We only translate the GFX ports #0 exposed to platform from conf data
  GfxSiliconPolicy[0].Power_State_Port = (PlatformGfxCfg->PowerOnPort0 != 0);

  return EFI_SUCCESS;
}

/**
  Helper function to apply GFX configuration data to GFX silicon policy.

  @param[in]      PolicyInterface   Pointer to current policy protocol/PPI interface.
  @param[in]      GfxConfigBuffer   Pointer to GFX configuration data.

  @retval EFI_SUCCESS           The configuration is translated to policy successfully.
  @retval EFI_INVALID_PARAMETER One or more of the required input pointers are NULL.
  @retval EFI_BUFFER_TOO_SMALL  Supplied GfxSiliconPolicy is too small to fit in translated data.
  @retval Others                Other errors occurred when getting GFX policy.
**/
EFI_STATUS
EFIAPI
ApplyGfxConfigToPolicy (
  IN  POLICY_PROTOCOL  *PolicyInterface,
  IN  VOID             *GfxConfigBuffer
  )
{
  EFI_STATUS  Status;
  UINT16      Size;
  UINT64      Attr = 0;

  GFX_CFG_DATA     *GfxConfData;
  GFX_POLICY_DATA  GfxSiPol[GFX_PORT_MAX_CNT];
  GFX_POLICY_DATA  GfxConfPol[GFX_PORT_MAX_CNT];

  if ((PolicyInterface == NULL) || (GfxConfigBuffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((DEBUG_ERROR, "%a Entry...\n", __FUNCTION__));

  GfxConfData = (GFX_CFG_DATA *)GfxConfigBuffer;
  Size        = sizeof (GfxSiPol);
  Status      = PolicyInterface->GetPolicy (&gPolicyDataGFXGuid, &Attr, GfxSiPol, &Size);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to get GFX policy - %r!!!\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Exit;
  }

  Size   = sizeof (GfxConfPol);
  Status = ConvertGfxPolicyFromConfData (PolicyInterface, GfxConfData, sizeof (GFX_CFG_DATA), GfxConfPol, &Size);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to convert GFX configuration to policy - %r!!!\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Exit;
  }

  if (CompareMem (GfxConfPol, GfxSiPol, Size) != 0) {
    Status = PolicyInterface->SetPolicy (&gPolicyDataGFXGuid, (Attr | POLICY_ATTRIBUTE_FINALIZED), GfxConfPol, Size);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a Failed to update GFX policy per configuration data - %r!!!\n", __FUNCTION__, Status));
      ASSERT (FALSE);
      goto Exit;
    }
  }

Exit:
  return Status;
}

/**
  Module entry point that will check configuration data and publish them to policy database.

  @param FileHandle                     The image handle.
  @param PeiServices                    The PEI services table.

  @retval Status                        From internal routine or boot object, should not fail
**/
EFI_STATUS
EFIAPI
ConfigDataGfxEntry (
  IN EFI_PEI_FILE_HANDLE     FileHandle,
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS                       Status;
  UINT8                            *ConfData = NULL;
  UINT32                           Attr      = 0;
  UINTN                            DataSize  = 0;
  UINTN                            Size      = 0;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *VarPpi   = NULL;
  POLICY_PPI                       *PolPpi   = NULL;
  CHAR16                           *UniName  = NULL;

  DEBUG ((DEBUG_INFO, "%a - Entry.\n", __FUNCTION__));

  // First locate policy ppi.
  Status = PeiServicesLocatePpi (&gPeiPolicyPpiGuid, 0, NULL, (VOID *)&PolPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to locate Policy PPI - %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    return Status;
  }

  // Publish GFX policy
  Status = PolPpi->SetPolicy (&gPolicyDataGFXGuid, 0, DefaultQemuGfxPolicy, sizeof (DefaultQemuGfxPolicy));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to set GFX policy - %r\n", __FUNCTION__, Status));
  }

  // Then locate variable ppi.
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID *)&VarPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "%a Failed to locate EFI_PEI_READ_ONLY_VARIABLE2_PPI, moving on with default policy - %r\n", __FUNCTION__, Status));
    Status = EFI_SUCCESS;
    goto Exit;
  }

  Size    = sizeof (SINGLE_SETTING_PROVIDER_START) + 8;
  UniName = AllocatePool (Size * 2);
  if (UniName == NULL) {
    DEBUG ((DEBUG_ERROR, "%a Failed to allocate buffer for GFX config data.\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  UnicodeSPrintAsciiFormat (UniName, Size * 2, SINGLE_SETTING_PROVIDER_TEMPLATE, CDATA_GFX_TAG);

  DataSize = 0;
  Status   = VarPpi->GetVariable (
                       VarPpi,
                       UniName,
                       &gSetupConfigPolicyVariableGuid,
                       &Attr,
                       &DataSize,
                       NULL
                       );
  if (Status == EFI_NOT_FOUND) {
    // This might be the first time, skip the rest
    Status = EFI_SUCCESS;
    goto Exit;
  }

  ConfData = AllocatePool (DataSize);
  Status   = VarPpi->GetVariable (
                       VarPpi,
                       UniName,
                       &gSetupConfigPolicyVariableGuid,
                       &Attr,
                       &DataSize,
                       ConfData
                       );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = ApplyGfxConfigToPolicy (PolPpi, ConfData);

Exit:
  return Status;
}
