/** @file
  QEMU FW CFG TPM Event Log Input Channel Library

  Allows a TPM replay log to be passed through the FW CFG interface on QEMU.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <IndustryStandard/QemuFwCfg.h>

#include <Library/DebugLib.h>
#include <Library/InputChannelLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/QemuFwCfgLib.h>

/**
  Retrieves a TPM Replay Event Log through a custom interface.

  This library instance returns a log from the QEMU FW CFG interface.
  https://www.qemu.org/docs/master/specs/fw_cfg.html

  @param[out] ReplayEventLog            A pointer to a pointer to the buffer to hold the event log data.
  @param[out] ReplayEventLogSize        The size of the data placed in the buffer.

  @retval    EFI_SUCCESS            The TPM Replay event log was returned successfully.
  @retval    EFI_INVALID_PARAMETER  A pointer argument given is NULL.
  @retval    EFI_UNSUPPORTED        The function is not implemented yet. The arguments are not used.
  @retval    EFI_COMPROMISED_DATA   The event log data found is not valid.
  @retval    EFI_NOT_FOUND          The event log data was not found.

**/
EFI_STATUS
EFIAPI
GetReplayEventLogFromCustomInterface (
  OUT VOID   **ReplayEventLog,
  OUT UINTN  *ReplayEventLogSize
  )
{
  EFI_STATUS            Status;
  FIRMWARE_CONFIG_ITEM  LogItem;
  UINTN                 LogSize;
  UINTN                 LogPageCount;
  VOID                  *LogBase;

  if ((ReplayEventLog == NULL) || (ReplayEventLogSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = QemuFwCfgFindFile ("opt/org.mu/tpm_replay/event_log", &LogItem, &LogSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[%a] - TPM Replay FW CFG event log not found (%r).\n", __func__, Status));
    return EFI_NOT_FOUND;
  }

  DEBUG ((DEBUG_INFO, "[%a] - TPM Replay FW CFG log found. Item 0x%x of size 0x%x.\n", __func__, LogItem, LogSize));

  LogPageCount = EFI_SIZE_TO_PAGES (LogSize);
  LogBase      = AllocatePages (LogPageCount);
  if (LogBase == NULL) {
    ASSERT (LogBase != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  QemuFwCfgSelectItem (LogItem);
  QemuFwCfgReadBytes (LogSize, LogBase);

  *ReplayEventLog     = LogBase;
  *ReplayEventLogSize = LogSize;

  return EFI_SUCCESS;
}
