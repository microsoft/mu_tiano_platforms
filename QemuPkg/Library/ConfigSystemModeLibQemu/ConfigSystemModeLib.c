/* @file ConfigSystemModeLib.c

  MFCI based library instance for system mode related functions for
  configuration modules on QEMU Q35 platform.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>

#include <Library/OemMfciLib.h>

/**
  This routine indicates if the system is in Manufacturing Mode.
  Platforms may have a manufacturing mode. Configuration update
  will only be allowed in such mode.

  @retval  TRUE   The device is in Manufacturing Mode.
  @retval  FALSE  The device is in Customer Mode.
**/
BOOLEAN
EFIAPI
IsSystemInManufacturingMode (
  VOID
  )
{
  return (GetMfciSystemOperationMode () == OEM_UEFI_MANUFACTURING_MODE);
}
