/** @file
  Common library interface for the autogen XML config header to call into to fetch a config value when the system
  is in MFCI manufacturing mode. In MFCI customer mode, the autogen XML config header contains the default values
  for each configuration profile and will return those values.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef CONFIG_KNOB_SHIM_LIB_COMMON_H_
#define CONFIG_KNOB_SHIM_LIB_COMMON_H_

#include <Uefi.h>

/**
  GetConfigKnobFromVariable returns the configuration knob from variable storage if it exists. This function is
  abstracted to work with both DXE and PEI.

  This function is only expected to be called by GetConfigKnob, via the autogen header code for config knobs.

  @param[in]  ConfigKnobGuid        The GUID of the requested config knob.
  @param[in]  ConfigKnobName        The name of the requested config knob.
  @param[out] ConfigKnobData        The retrieved data of the requested config knob. The caller will allocate memory for
                                    this buffer and is responsible for freeing it.
  @param[in out] ConfigKnobDataSize The allocated size of ConfigKnobData. This is expected to be set to the correct
                                    value for the size of ConfigKnobData. If this size is too small for the config knob,
                                    EFI_BUFFER_TOO_SMALL will be returned. This represents a mismatch in the profile
                                    expected size and what is stored in variable storage, so the profile value will
                                    take precedence.

  @retval EFI_NOT_FOUND           The requested config knob was not found in the policy cache or variable storage. This
                                  is expected when the config knob has not been updated from the profile default.
  @retval EFI_BUFFER_TOO_SMALL    ConfigKnobDataSize as passed in was too small for this config knob. This is expected
                                  if stale variables exist in flash.
  @retval !EFI_SUCCESS            Failed to read variable from variable storage.
  @retval EFI_SUCCESS             The operation succeeds.

**/
EFI_STATUS
GetConfigKnobFromVariable (
  IN EFI_GUID   *ConfigKnobGuid,
  IN CHAR16     *ConfigKnobName,
  OUT VOID      *ConfigKnobData,
  IN OUT UINTN  *ConfigKnobDataSize
  );

#endif // CONFIG_KNOB_SHIM_LIB_COMMON_H_
