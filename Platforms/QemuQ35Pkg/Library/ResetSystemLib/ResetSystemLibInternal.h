/** @file
  Local header file with internal library internal functionality.

  Copyright (c) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patents

**/

#ifndef RESET_SYSTEM_LIB_INTERNAL_H_
#define RESET_SYSTEM_LIB_INTERNAL_H_

/**
  Stores hardware reset context for later use.

  @retval EFI_SUCCESS     The hardware information was saved successfully.
  @retval EFI_UNSUPPORTED The host bridge PCI device ID is invalid or unexpected.

**/
EFI_STATUS
SaveHardwareContext (
  VOID
  );

#endif
