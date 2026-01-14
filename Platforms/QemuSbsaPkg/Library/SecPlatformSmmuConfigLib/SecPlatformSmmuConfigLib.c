/** @file SecPlatformSmmuConfigLib.c

  This file contains a utility function to build the SMMU_CONFIG HOB.
  It takes the platform specific IORT and builds the SMMU_CONFIG HOB.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Guid/SmmuConfig.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SecPlatformSmmuConfigLib.h>
#include "Iort.aslc"

/**
  Builds SMMU_CONFIG HOB for SmmuDxe to consume.

  This HOB contains the actual IORT table data for SMMU configuration.
  Do not install the IORT yourself, SmmuDxe will do that.

  @retval EFI_SUCCESS               SmmuConfig HOB is built successfully.
  @retval EFI_NOT_FOUND             Failed to find RMR range for IORT.
  @retval EFI_OUT_OF_RESOURCES      Failed to allocate memory for SMMU_CONFIG HOB.
**/
EFI_STATUS
EFIAPI
BuildSmmuConfigHob (
  VOID
  )
{
  SMMU_CONFIG  *SmmuConfig;

  SmmuConfig               = (SMMU_CONFIG *)AllocateZeroPool (sizeof (SMMU_CONFIG) + sizeof (IortData));
  SmmuConfig->VersionMajor = CURRENT_SMMU_CONFIG_VERSION_MAJOR;
  SmmuConfig->VersionMinor = CURRENT_SMMU_CONFIG_VERSION_MINOR;
  SmmuConfig->IortSize     = sizeof (IortData);    // Size of the IORT table in the structure
  SmmuConfig->IortOffset   = sizeof (SMMU_CONFIG); // Offset of the IORT table in the structure

  CopyMem ((VOID *)((UINT8 *)SmmuConfig + SmmuConfig->IortOffset), &IortData, sizeof (IortData));

  BuildGuidDataHob (&gSmmuConfigHobGuid, SmmuConfig, sizeof (SMMU_CONFIG) + sizeof (IortData));

  FreePool (SmmuConfig);

  DEBUG ((DEBUG_VERBOSE, "%a: Configured SmmuConfig Hob.\n", __func__));
  return EFI_SUCCESS;
}
