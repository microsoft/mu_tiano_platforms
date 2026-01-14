/** @file SecPlatformSmmuConfigLib.h

    Copyright (c) Microsoft Corporation.
    SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef SEC_PLATFORM_SMMU_CONFIG_LIB_H_
#define SEC_PLATFORM_SMMU_CONFIG_LIB_H_

#include <Uefi/UefiBaseType.h>

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
  );

#endif // SEC_PLATFORM_SMMU_CONFIG_LIB_H_
