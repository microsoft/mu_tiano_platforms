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

#define FFA_NOTIFICATION_SERVICE_GUID \
  { \
    0xb510b3a3, 0x59f6, 0x4054, { 0xba, 0x7a, 0xff, 0x2e, 0xb1, 0xea, 0xc7, 0x65 } \
  }

#define FFA_TPM_SERVICE_GUID \
  { \
    0x17b862a4, 0x1806, 0x4faf, { 0x86, 0xb3, 0x08, 0x9a, 0x58, 0x35, 0x38, 0x61 } \
  }

#define FFA_TEST_SERVICE_GUID \
  { \
    0xe0fad9b3, 0x7f5c, 0x42c5, { 0xb2, 0xee, 0xb7, 0xa8, 0x23, 0x13, 0xcd, 0xb2 } \
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
  ARM_SMC_ARGS            SmcArgs                    = { 0 };
  EFI_GUID                FfaTestServiceGuid         = FFA_TEST_SERVICE_GUID;
  EFI_GUID                FfaNotificationServiceGuid = FFA_NOTIFICATION_SERVICE_GUID;
  EFI_GUID                FfaTpmServiceGuid          = FFA_TPM_SERVICE_GUID;
  EFI_FFA_PART_INFO_DESC  FfaTestPartInfo;
  UINT32                  Count;
  UINT32                  Size;
  UINTN                   SriIndex;
  UINTN                   Dummy;
  DIRECT_MSG_ARGS_EX      DirectMsgArgsEx;
  UINT16                  CurrentMajorVersion;
  UINT16                  CurrentMinorVersion;

  // Query FF-A version to make sure FF-A is supported
  Status = ArmFfaLibGetVersion (
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

  // Retrieve the partition information from the returned registers
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

  // Retrieve the partition information from the returned registers
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
  Status = ArmFfaLibGetFeatures (ARM_FFA_FEATURE_ID_SCHEDULE_RECEIVER_INTERRUPT, 0, &SriIndex, &Dummy);
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
  FfaNotificationBind (FfaTestPartInfo.PartitionId, 0, 0x4);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to bind notification with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  // Setup the Thermal Service Notification Bits
  ZeroMem (&DirectMsgArgsEx, sizeof (DirectMsgArgsEx));
  DirectMsgArgsEx.Arg0 = 0x01; // Setup
  DirectMsgArgsEx.Arg1 = 0xba7aff2eb1eac765;
  DirectMsgArgsEx.Arg2 = 0xb610b3a359f64054;
  DirectMsgArgsEx.Arg3 = 0x03;
  DirectMsgArgsEx.Arg4 = ((6 << 16) | (0));
  DirectMsgArgsEx.Arg5 = ((7 << 16) | (1));
  DirectMsgArgsEx.Arg6 = ((8 << 16) | (2));
  Status               = FfaMessageSendDirectReq2 (FfaTestPartInfo.PartitionId, &FfaNotificationServiceGuid, &DirectMsgArgsEx);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to communicate direct req 2 with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  if (DirectMsgArgsEx.Arg0 != 0) {
    DEBUG ((DEBUG_ERROR, "Command Failed: %x\n", DirectMsgArgsEx.Arg0));
    goto Done;
  } else {
    DEBUG ((DEBUG_INFO, "Thermal Service Setup Success\n"));
  }

  // Setup the Battery Service Notification Bits
  ZeroMem (&DirectMsgArgsEx, sizeof (DirectMsgArgsEx));
  DirectMsgArgsEx.Arg0 = 0x01; // Setup
  DirectMsgArgsEx.Arg1 = 0xba7aff2eb1eac765;
  DirectMsgArgsEx.Arg2 = 0xb710b3a359f64054;
  DirectMsgArgsEx.Arg3 = 0x05;
  DirectMsgArgsEx.Arg4 = ((1 << 16) | (0));
  DirectMsgArgsEx.Arg5 = ((2 << 16) | (1));
  DirectMsgArgsEx.Arg6 = ((3 << 16) | (2));
  DirectMsgArgsEx.Arg7 = ((4 << 16) | (3));
  DirectMsgArgsEx.Arg8 = ((5 << 16) | (4));
  Status               = FfaMessageSendDirectReq2 (FfaTestPartInfo.PartitionId, &FfaNotificationServiceGuid, &DirectMsgArgsEx);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to communicate direct req 2 with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  if (DirectMsgArgsEx.Arg0 != 0) {
    DEBUG ((DEBUG_ERROR, "Command Failed: %x\n", DirectMsgArgsEx.Arg0));
    goto Done;
  } else {
    DEBUG ((DEBUG_INFO, "Battery Service Setup Success\n"));
  }

  // Destroy the Thermal Service Notification Bit 1
  ZeroMem (&DirectMsgArgsEx, sizeof (DirectMsgArgsEx));
  DirectMsgArgsEx.Arg0 = 0x02; // Destroy
  DirectMsgArgsEx.Arg1 = 0xba7aff2eb1eac765;
  DirectMsgArgsEx.Arg2 = 0xb610b3a359f64054;
  DirectMsgArgsEx.Arg3 = 0x01;
  DirectMsgArgsEx.Arg4 = ((7 << 16) | (1));
  Status               = FfaMessageSendDirectReq2 (FfaTestPartInfo.PartitionId, &FfaNotificationServiceGuid, &DirectMsgArgsEx);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to communicate direct req 2 with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  if (DirectMsgArgsEx.Arg0 != 0) {
    DEBUG ((DEBUG_ERROR, "Command Failed: %x\n", DirectMsgArgsEx.Arg0));
    goto Done;
  } else {
    DEBUG ((DEBUG_INFO, "Thermal Service Destroy Success\n"));
  }

  // Call the TPM Service get_interface_version
  ZeroMem (&DirectMsgArgsEx, sizeof (DirectMsgArgsEx));
  DirectMsgArgsEx.Arg0 = 0x0F000001;
  Status               = FfaMessageSendDirectReq2 (FfaTestPartInfo.PartitionId, &FfaTpmServiceGuid, &DirectMsgArgsEx);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to communicate direct req 2 with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  if (DirectMsgArgsEx.Arg0 != 0x05000002) {
    DEBUG ((DEBUG_ERROR, "Command Failed: %x\n", DirectMsgArgsEx.Arg0));
    goto Done;
  } else {
    DEBUG ((DEBUG_INFO, "TPM Service Interface Version: %d.%d\n", DirectMsgArgsEx.Arg1 >> 16, DirectMsgArgsEx.Arg1 & 0xFFFF));
  }

  // Invoke the Test Service to trigger a notification event
  ZeroMem (&DirectMsgArgsEx, sizeof (DirectMsgArgsEx));
  DirectMsgArgsEx.Arg0 = 0xDEF1;
  DirectMsgArgsEx.Arg1 = 0xba7aff2eb1eac765;
  DirectMsgArgsEx.Arg2 = 0xb710b3a359f64054; // Battery Service
  DirectMsgArgsEx.Arg3 = 0x01;               // ID 1
  Status               = FfaMessageSendDirectReq2 (FfaTestPartInfo.PartitionId, &FfaTestServiceGuid, &DirectMsgArgsEx);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to communicate direct req 2 with FF-A Ffa test SP (%r).\n", Status));
    goto Done;
  }

  if (DirectMsgArgsEx.Arg0 != 0) {
    DEBUG ((DEBUG_ERROR, "Command Failed: %x\n", DirectMsgArgsEx.Arg0));
    goto Done;
  } else {
    DEBUG ((DEBUG_INFO, "Test Test Service Notification Test Success\n"));
  }

  return EFI_SUCCESS;

Done:
  return Status;
}
