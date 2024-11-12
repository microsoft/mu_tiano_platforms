/** @file
  Test Secure Partition

  Copyright (c) 2009 - 2014, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2016 - 2021, Arm Limited. All rights reserved.<BR>
  Copyright (c), Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiMm.h>
#include <Base.h>
#include <IndustryStandard/ArmFfaSvc.h>

#include <Library/StandaloneMmCoreEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/ArmSvcLib.h>
#include <Library/ArmFfaLib.h>
#include <Library/ArmFfaLibEx.h>

#define SENDER_ID(x)    (x >> 16) & 0xffff
#define RECEIVER_ID(x)  (x & 0xffff)

volatile BOOLEAN  loop = TRUE;

/**
  The Entry Point for Test Secure Partition.

  @param  HobStart       Pointer to the start of the HOB list.

  @retval EFI_SUCCESS             Success.
  @retval EFI_UNSUPPORTED         Unsupported operation.
**/
EFI_STATUS
EFIAPI
ExampleSecurePartitionMain (
  IN VOID  *HobStart
  )
{
  EFI_STATUS          Status;
  DIRECT_MSG_ARGS_EX  payload;
  UINT16              TargetId;
  UINT16              MyId;

  DEBUG ((DEBUG_INFO, "%a - 0x%x\n", __func__, HobStart));

  DUMP_HEX (DEBUG_ERROR, 0, 0x000001001FFFE000, EFI_PAGE_SIZE, "");

  Status = FfaMessageWait (&payload);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to wait for message %r\n", Status));
    while (loop) {
    }
  }

  // print the incoming message
  DEBUG ((DEBUG_INFO, "\tReceived message fid=0x%x, sender=0x%x\n", payload.FunctionId, payload.SourceId));
  DEBUG ((DEBUG_INFO, "\tReceived message receiver=0x%x, 0x%x\n", payload.DestinationId, payload.DestinationId));

  TargetId = payload.SourceId;
  MyId     = payload.DestinationId;

  ZeroMem (&payload, sizeof (DIRECT_MSG_ARGS_EX));
  payload.SourceId      = MyId;
  payload.DestinationId = TargetId;
  payload.Arg0          = 0;
  payload.Arg1          = 0;
  payload.Arg2          = 0xdeadbeef;
  Status                = FfaMessageSendDirectResp64 (&payload, &payload);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to send response 64 %r\n", Status));
    while (loop) {
    }
  }

  // print the incoming message
  DEBUG ((DEBUG_INFO, "\tReceived 2nd message fid=0x%x, sender=0x%x\n", payload.FunctionId, payload.SourceId));
  DEBUG ((DEBUG_INFO, "\tReceived 2nd message receiver=0x%x, 0x%x\n", payload.DestinationId, payload.DestinationId));

  TargetId = payload.SourceId;
  MyId     = payload.DestinationId;

  ZeroMem (&payload, sizeof (DIRECT_MSG_ARGS_EX));
  payload.SourceId      = MyId;
  payload.DestinationId = TargetId;
  payload.Arg0          = 0;
  payload.Arg1          = 0;
  payload.Arg2          = 0xfeedf00d;
  Status                = FfaMessageSendDirectResp2 (&payload, &payload);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to send response 2 %r\n", Status));
    while (loop) {
    }
  }

  // print the incoming message
  DEBUG ((DEBUG_INFO, "\tReceived 3rd message fid=0x%x, sender=0x%x\n", payload.FunctionId, payload.SourceId));
  DEBUG ((DEBUG_INFO, "\tReceived 3rd message receiver=0x%x, 0x%x\n", payload.DestinationId, payload.DestinationId));

  // Intended Feature 2: Register interrupt handler for the intended GPIO
  // TODO: Implement this feature

  // Intended Feature 3: send notification to the VM0
  // send a message to whomever sent the message
  Status = FfaNotificationSet (TargetId, FFA_NOTIFICATIONS_FLAG_DELAY_SRI | FFA_NOTIFICATIONS_FLAG_PER_VCPU, 1);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed notification set err %r\n", Status));
    while (loop) {
    }
  }

  DEBUG ((DEBUG_ERROR, "Done setting notifications!!\n"));

  ZeroMem (&payload, sizeof (DIRECT_MSG_ARGS_EX));
  payload.SourceId      = MyId;
  payload.DestinationId = TargetId;
  payload.Arg0          = 0;
  payload.Arg1          = 0;
  payload.Arg2          = 0xba5eba11;
  Status                = FfaMessageSendDirectResp2 (&payload, &payload);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to again send response 2 %r\n", Status));
    while (loop) {
    }
  }

  while (1) {
    ZeroMem (&payload, sizeof (DIRECT_MSG_ARGS_EX));
    FfaMessageWait (&payload);

    if (payload.FunctionId != ARM_FID_FFA_MSG_SEND_DIRECT_REQ_AARCH64) {
      DEBUG ((DEBUG_ERROR, "\tInvalid fid received, fid=0x%x, error=0x%x\n", payload.FunctionId, payload.Arg0));
      while (loop) {
      }
    }

    TargetId = payload.DestinationId;
    MyId     = payload.SourceId;

    ZeroMem (&payload, sizeof (DIRECT_MSG_ARGS_EX));
    payload.SourceId      = MyId;
    payload.DestinationId = TargetId;
    payload.Arg0          = 1;    // VAL_ERROR
    FfaMessageSendDirectResp2 (&payload, &payload);
  }

  DEBUG ((DEBUG_INFO, "%a - Done!\n", __func__));

  return EFI_SUCCESS;
}
