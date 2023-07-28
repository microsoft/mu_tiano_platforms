/** @file
  Standalone MM Reset System Library Shutdown API implementation for QEMU.

  Copyright (C) 2020, Red Hat, Inc.
  Copyright (c) 2006 - 2019, Intel Corporation. All rights reserved.<BR>
  Copyright (c) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiMm.h>
#include "ResetSystemLibInternal.h"

/**
  Constructor for the Standalone MM library instance.

  @param[in] ImageHandle  Image handle of this driver.
  @param[in] SystemTable  A Pointer to the MM System Table.

  @retval EFI_SUCEESS     Always returns success.

**/
EFI_STATUS
EFIAPI
StandaloneMmConstructor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_MM_SYSTEM_TABLE  *SystemTable
  )
{
  SaveHardwareContext ();

  return EFI_SUCCESS;
}
