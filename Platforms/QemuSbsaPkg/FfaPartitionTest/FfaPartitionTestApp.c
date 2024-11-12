/** @file
FfaPartitionTestApp.c

This is a Unit Test for the test FFA service library.

Copyright (C) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <IndustryStandard/ArmFfaSvc.h>
#include <IndustryStandard/ArmFfaBootInfo.h>
#include <IndustryStandard/ArmFfaPartInfo.h>
#include <Pi/PiMultiPhase.h>
#include <Protocol/HardwareInterrupt.h>

#include <Library/ArmSmcLib.h>
#include <Library/ArmSvcLib.h>
#include <Library/ArmLib.h>
#include <Library/ArmFfaLib.h>
#include <Library/ArmFfaLibEx.h>
#include <Library/ArmGicLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#define FFA_TEST_SERVICE_GUID \
  { \
    0x7036A3DB, 0xBA3B, 0x44E5, { 0xB3, 0x59, 0xE2, 0x5E, 0x5D, 0x90, 0x43, 0x87 } \
  }

UINT16  FfaPartId;

EFI_HARDWARE_INTERRUPT_PROTOCOL  *gInterrupt;

/**
  EFI_CPU_INTERRUPT_HANDLER that is called when a processor interrupt occurs.

  @param  InterruptType    Defines the type of interrupt or exception that
                           occurred on the processor. This parameter is
                           processor architecture specific.
  @param  SystemContext    A pointer to the processor context when
                           the interrupt occurred on the processor.

  @return None

**/
VOID
EFIAPI
ApIrqInterruptHandler (
  IN  HARDWARE_INTERRUPT_SOURCE  Source,
  IN  EFI_SYSTEM_CONTEXT         SystemContext
  )
{
  EFI_STATUS  Status;
  UINT64      Bitmap;

  DEBUG ((DEBUG_INFO, "Received IRQ interrupt %d!\n", Source));

  // Then register this test app to receive notifications from the Ffa test SP
  Status = FfaNotificationGet (0, FFA_NOTIFICATIONS_FLAG_BITMAP_SP, &Bitmap);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to notification get with FF-A Ffa test SP (%r).\n", Status));
  } else {
    DEBUG ((DEBUG_INFO, "Got notification from FF-A Ffa test SP with VM bitmap %x.\n", Bitmap));
  }

  gInterrupt->EndOfInterrupt (gInterrupt, Source);
}

/**
  FfaPartitionTestAppEntry

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point executed successfully.
  @retval other           Some error occurred when executing this entry point.

**/
EFI_STATUS
EFIAPI
FfaPartitionTestAppEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS              Status;
  ARM_SMC_ARGS            SmcArgs            = { 0 };
  EFI_GUID                FfaTestServiceGuid = FFA_TEST_SERVICE_GUID;
  EFI_FFA_PART_INFO_DESC  FfaTestPartInfo;
  UINT32                  Count;
  UINT32                  Size;
  UINTN                   SriIndex;
  UINTN                   Dummy;
  DIRECT_MSG_ARGS_EX      DirectMsgArgsEx;
  DIRECT_MSG_ARGS         DirectMsgArgs;
  UINT16                  CurrentMajorVersion;
  UINT16                  CurrentMinorVersion;

  // Query FF-A version to make sure FF-A is supported
  Status = ArmFfaLibVersion (
             ARM_FFA_MAJOR_VERSION,
             ARM_FFA_MINOR_VERSION,
             &CurrentMajorVersion,
             &CurrentMinorVersion
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to get FF-A version. Status: %r\n", Status));
    goto Done;
  }

  // FF-A is supported, then discover the Ffa Test SP's presence, ID, our ID and
  // retrieve our RX/TX buffers.
  Status = EFI_UNSUPPORTED;

  // Get our ID
  Status = ArmFfaLibPartitionIdGet (&FfaPartId);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to retrieve FF-A partition ID (%r).\n", Status));
    goto Done;
  }

  // Discover the Ffa all SPs through registers.
  ZeroMem (&SmcArgs, sizeof (SmcArgs));
  Count  = 5;
  Status = FfaPartitionInfoGetRegs (NULL, 0, NULL, &Count, (EFI_FFA_PART_INFO_DESC *)&SmcArgs.Arg3);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to discover FF-A all SP (%r).\n", Status));
    goto Done;
  }

  DUMP_HEX (DEBUG_INFO, 0, &SmcArgs, sizeof (SmcArgs), "    ");

  // Retrieve the partition information from the retuend registers
  CopyMem (&FfaTestPartInfo, &SmcArgs.Arg3, sizeof (EFI_FFA_PART_INFO_DESC));

  DEBUG ((DEBUG_INFO, "Discovered first FF-A Ffa SP.\n"));
  DEBUG ((
    DEBUG_INFO,
    "\tID = 0x%lx, Execution contexts = %d, Properties = 0x%lx.\n",
    FfaTestPartInfo.PartitionId,
    FfaTestPartInfo.ExecContextCountOrProxyPartitionId,
    FfaTestPartInfo.PartitionProps
    ));
  DEBUG ((
    DEBUG_INFO,
    "\tSP Guid = %g.\n",
    FfaTestPartInfo.PartitionUuid
    ));

  // Discover the Ffa test SP after converting the EFI_GUID to a format TF-A will
  // understand.
  Status = ArmFfaLibPartitionInfoGet (&FfaTestServiceGuid, 0, &Count, &Size);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to discover FF-A test SP (%r).\n", Status));
    goto Done;
  }

  // Retrieve the partition information from the retuend registers
  CopyMem (&FfaTestPartInfo, (VOID *)PcdGet64 (PcdFfaRxBuffer), sizeof (EFI_FFA_PART_INFO_DESC));

  DEBUG ((DEBUG_INFO, "Discovered FF-A test SP.\n"));
  DEBUG ((
    DEBUG_INFO,
    "\tID = 0x%lx, Execution contexts = %d, Properties = 0x%lx.\n",
    FfaTestPartInfo.PartitionId,
    FfaTestPartInfo.ExecContextCountOrProxyPartitionId,
    FfaTestPartInfo.PartitionProps
    ));
  DEBUG ((
    DEBUG_INFO,
    "\tSP Guid = %g.\n",
    FfaTestPartInfo.PartitionUuid
    ));

  //
  // Should be able to handle notification flow now.
  //
  // Now register UEFI to receive notifications by creating notification bitmaps
  Status = FfaNotificationBitmapCreate (1);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to create notification bitmap with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  // Followed by querying which notification ID is supported by the Ffa test SP
  Status = ArmFfaLibFeatures (ARM_FFA_FEATURE_ID_SCHEDULE_RECEIVER_INTERRUPT, 0, &SriIndex, &Dummy);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to query feature SRI number with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  DEBUG ((DEBUG_ERROR, "Received feature SRI number with FF-A Ffa test SP (%d).\n", SriIndex));

  // Register the IRQ handler for the FF-A Ffa test SP
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  if (!EFI_ERROR (Status)) {
    Status = gInterrupt->RegisterInterruptSource (gInterrupt, SriIndex, ApIrqInterruptHandler);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Unable to register notification (%r).\n", Status));
      goto Done;
    }
  }

  // Then register this test app to receive notifications from the Ffa test SP
  FfaNotificationBind (FfaTestPartInfo.PartitionId, FFA_NOTIFICATIONS_FLAG_PER_VCPU, 0x1);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to bind notification with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  // Next, communicate to the Ffa test SP
  ZeroMem (&DirectMsgArgs, sizeof (DirectMsgArgs));
  Status = ArmFfaLibMsgDirectReq (FfaTestPartInfo.PartitionId, 0, &DirectMsgArgs);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to communicate with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  DUMP_HEX (DEBUG_INFO, 0, &DirectMsgArgs, sizeof (DirectMsgArgs), "    ");

  // Communicate to the Ffa test SP with direct message req 2, twice
  // This will make the Ffa test SP send a notification to the UEFI through SRI
  ZeroMem (&DirectMsgArgsEx, sizeof (DirectMsgArgsEx));
  Status = FfaMessageSendDirectReq2 (FfaTestPartInfo.PartitionId, NULL, &DirectMsgArgsEx);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to communicate direct req 2 with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  DUMP_HEX (DEBUG_INFO, 0, &DirectMsgArgsEx, sizeof (DirectMsgArgsEx), "    ");

  // Next, communicate to the Ffa test SP
  // This will make the Ffa test SP send a notification to the UEFI through SRI
  ZeroMem (&DirectMsgArgsEx, sizeof (DirectMsgArgsEx));
  Status = FfaMessageSendDirectReq2 (FfaTestPartInfo.PartitionId, NULL, &DirectMsgArgsEx);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to communicate direct req 2 again with FF-A Ffa test SP (%r).\n", Status));
  }

  DUMP_HEX (DEBUG_INFO, 0, &DirectMsgArgsEx, sizeof (DirectMsgArgsEx), "    ");

  return EFI_SUCCESS;

Done:
  return Status;
}
