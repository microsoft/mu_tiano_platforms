/** @file
  Microsoft Secure Partition

  Copyright (c), Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

// Secure Partition Headers
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
#include <Library/NotificationServiceLib.h>
#include <Library/TestServiceLib.h>
#include <Library/TpmServiceLib.h>
#include <Guid/Tpm2ServiceFfa.h>
#include <Guid/NotificationServiceFfa.h>
#include <Guid/TestServiceFfa.h>

/**
  Message Handler for the Microsoft Secure Partition

  @param  Request       The incoming message
  @param  Response      The outgoing message

**/
STATIC
VOID
EFIAPI
MsSecurePartitionHandleMessage (
  DIRECT_MSG_ARGS_EX  *Request,
  DIRECT_MSG_ARGS_EX  *Response
  )
{
  ZeroMem (Response, sizeof (DIRECT_MSG_ARGS_EX));
  Response->SourceId      = Request->DestinationId;
  Response->DestinationId = Request->SourceId;

  if (!CompareMem (&Request->ServiceGuid, &gEfiNotificationServiceFfaGuid, sizeof (EFI_GUID))) {
    NotificationServiceHandle (Request, Response);
  } else if (!CompareMem (&Request->ServiceGuid, &gEfiTpm2ServiceFfaGuid, sizeof (EFI_GUID))) {
#ifdef TPM2_ENABLE
    TpmServiceHandle (Request, Response);
#else
    Response->Arg0 = EFI_UNSUPPORTED;
#endif
  } else if (!CompareMem (&Request->ServiceGuid, &gEfiTestServiceFfaGuid, sizeof (EFI_GUID))) {
    TestServiceHandle (Request, Response);
  } else {
    DEBUG ((DEBUG_ERROR, "Invalid secure partition service UUID\n"));
    Response->Arg0 = EFI_NOT_FOUND;
  }
}

/**
  The Entry Point for Microsoft Secure Partition.

  @param  HobStart       Pointer to the start of the HOB list.

  @retval EFI_SUCCESS             Success.
  @retval EFI_UNSUPPORTED         Unsupported operation.
**/
EFI_STATUS
EFIAPI
MsSecurePartitionMain (
  IN VOID  *HobStart
  )
{
  EFI_STATUS          Status;
  DIRECT_MSG_ARGS_EX  Request;
  DIRECT_MSG_ARGS_EX  Response;

  // Initialize the services running in this secure partition
  NotificationServiceInit ();
#ifdef TPM2_ENABLE
  TpmServiceInit ();
  #error "TPM2_ENABLE is defined"
#endif
  TestServiceInit ();

  DEBUG ((DEBUG_INFO, "MS-Services secure partition initialized and running!\n"));

  Status = FfaMessageWait (&Request);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to wait for message %r\n", Status));
    ASSERT (FALSE);
  }

  while (1) {
    MsSecurePartitionHandleMessage (&Request, &Response);

    Status = FfaMessageSendDirectResp2 (&Response, &Request);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to send direct response %r\n", Status));
      Status = FfaMessageWait (&Request);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Failed to wait for message %r\n", Status));
        ASSERT (FALSE);
      }
    }
  }

  DEBUG ((DEBUG_ERROR, "Reached the end of %a - Invalid!\n", __func__));
  return EFI_SUCCESS;
}
