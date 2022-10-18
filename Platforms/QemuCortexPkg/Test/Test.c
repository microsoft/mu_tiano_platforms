/** @file
  Test module for QEMU cortex package.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include <Library/DebugLib.h>

/**
  Module entry point that will do basic test for QEMU cortex package.

  @param ImageHandle                    The image handle.
  @param SystemTable                    The DXE system table.

  @retval Status                        From internal routine or boot object, should not fail
**/
EFI_STATUS
EFIAPI
TestEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DEBUG ((DEBUG_INFO, "%a - Entry.\n", __FUNCTION__));

  DEBUG ((DEBUG_INFO, "################################################################\n", __FUNCTION__));
  DEBUG ((DEBUG_INFO, "#                                                              #\n", __FUNCTION__));
  DEBUG ((DEBUG_INFO, "#                          TEST DRIVER                         #\n", __FUNCTION__));
  DEBUG ((DEBUG_INFO, "#                                                              #\n", __FUNCTION__));
  DEBUG ((DEBUG_INFO, "################################################################\n", __FUNCTION__));

  DEBUG ((DEBUG_INFO, "%a - Exit.\n", __FUNCTION__));

  return EFI_SUCCESS;
}
