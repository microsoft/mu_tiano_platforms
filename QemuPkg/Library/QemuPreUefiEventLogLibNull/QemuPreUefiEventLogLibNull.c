/** @file
  NULL implementation of PreUefiEventLogLib for QEMU.

  Copyright (c) Microsoft Corporation
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>

/**
  Create the event log entries, Nothing to do for QEMU.
**/
VOID
CreateTcg2PreUefiEventLogEntries (
  VOID
  )
{
  return;
}
