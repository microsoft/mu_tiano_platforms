/* @file Q35ConfigDataLib.c

  Library to supply platform data to OemConfigPolicyCreatorPei.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <ConfigStdStructDefs.h>
#define CONFIG_INCLUDE_CACHE
#include <Generated/ConfigClientGenerated.h>
#include <Generated/ConfigDataGenerated.h>
#include <Generated/ConfigProfilesGenerated.h>
