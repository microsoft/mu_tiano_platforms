/** @file
  Entry point to the Standalone MM Foundation when initialized during the SEC
  phase on ARM platforms

Copyright (c) 2017 - 2021, Arm Ltd. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiMm.h>

#include <PiPei.h>
#include <Guid/MmramMemoryReserve.h>
#include <Guid/MpInformation.h>

#include <Library/ArmStandaloneMmCoreEntryPoint.h>
#include <Library/ArmSvcLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SerialPortLib.h>
#include <Library/ArmStandaloneMmMmuLib.h>
#include <Library/PcdLib.h>

#include <IndustryStandard/ArmStdSmc.h>
#include <IndustryStandard/ArmMmSvc.h>
#include <IndustryStandard/ArmFfaSvc.h>

#include <Protocol/MmCommunication.h>
#include <Protocol/PiMmCpuDriverEp.h>

#include "StandaloneMmCoreEntryPoint.h"

extern EFI_MM_SYSTEM_TABLE  gMmCoreMmst;

#define BOOT_PAYLOAD_VERSION  1

extern VOID  *gHobList;

STATIC MISC_MM_COMMUNICATE_BUFFER  *mMiscMmCommunicateBuffer = NULL;
STATIC EFI_MMRAM_DESCRIPTOR        *mNsCommBuffer            = NULL;
STATIC EFI_MMRAM_DESCRIPTOR        *mSCommBuffer             = NULL;

/**
  Get communication ABI protocol.

  @param  [out]   CommProtocol       Communication protocol.

  @retval         EFI_SUCCESS
  @retval         EFI_UNSUPPORTED    Not supported

**/
STATIC
EFI_STATUS
EFIAPI
GetCommProtocol (
  OUT COMM_PROTOCOL  *CommProtocol
  )
{
  EFI_STATUS    Status;
  UINT16        RequestMajorVersion;
  UINT16        RequestMinorVersion;
  UINT16        CurrentMajorVersion;
  UINT16        CurrentMinorVersion;
  ARM_SVC_ARGS  SvcArgs;

  RequestMajorVersion = ARM_FFA_MAJOR_VERSION;
  RequestMinorVersion = ARM_FFA_MINOR_VERSION;

  Status = ArmFfaLibGetVersion (
             RequestMajorVersion,
             RequestMinorVersion,
             &CurrentMajorVersion,
             &CurrentMinorVersion
             );
  if (!EFI_ERROR (Status)) {
    *CommProtocol = CommProtocolFfa;
  } else {
    ZeroMem (&SvcArgs, sizeof (ARM_SVC_ARGS));
    SvcArgs.Arg0 = ARM_FID_SPM_MM_VERSION_AARCH32;

    ArmCallSvc (&SvcArgs);

    if (SvcArgs.Arg0 == ARM_SPM_MM_RET_NOT_SUPPORTED) {
      *CommProtocol = CommProtocolUnknown;
      return EFI_UNSUPPORTED;
    }

    *CommProtocol       = CommProtocolSpmMm;
    RequestMajorVersion = ARM_SPM_MM_SUPPORT_MAJOR_VERSION;
    RequestMinorVersion = ARM_SPM_MM_SUPPORT_MINOR_VERSION;
    CurrentMajorVersion =
      ((SvcArgs.Arg0 >> ARM_SPM_MM_MAJOR_VERSION_SHIFT) & ARM_SPM_MM_VERSION_MASK);
    CurrentMinorVersion =
      ((SvcArgs.Arg0 >> ARM_SPM_MM_MINOR_VERSION_SHIFT) & ARM_SPM_MM_VERSION_MASK);
  }

  // Different major revision values indicate possibly incompatible functions.
  // For two revisions, A and B, for which the major revision values are
  // identical, if the minor revision value of revision B is greater than
  // the minor revision value of revision A, then every function in
  // revision A must work in a compatible way with revision B.
  // However, it is possible for revision B to have a higher
  // function count than revision A
  if ((RequestMajorVersion != CurrentMajorVersion) ||
      (RequestMinorVersion > CurrentMinorVersion))
  {
    DEBUG ((
      DEBUG_INFO,
      "Incompatible %s Versions.\n" \
      "Request Version: Major=0x%x, Minor>=0x%x.\n" \
      "Current Version: Major=0x%x, Minor=0x%x.\n",
      (*CommProtocol == CommProtocolFfa) ? L"FF-A" : L"SPM_MM",
      RequestMajorVersion,
      RequestMinorVersion,
      CurrentMajorVersion,
      CurrentMinorVersion
      ));

    return EFI_UNSUPPORTED;
  }

  DEBUG ((
    DEBUG_INFO,
    "%s Version: Major=0x%x, Minor=0x%x\n",
    (*CommProtocol == CommProtocolFfa) ? L"FF-A" : L"SPM_MM",
    CurrentMajorVersion,
    CurrentMinorVersion
    ));

  return EFI_SUCCESS;
}

/**
  Retrieve a pointer to and print the boot information passed by privileged
  secure firmware.

  @param  [in] SharedBufAddress   The pointer memory shared with privileged
                                  firmware.

**/
EFI_SECURE_PARTITION_BOOT_INFO *
GetAndPrintBootinformation (
  IN VOID  *SharedBufAddress
  )
{
  EFI_SECURE_PARTITION_BOOT_INFO  *PayloadBootInfo;
  EFI_SECURE_PARTITION_CPU_INFO   *PayloadCpuInfo;
  UINTN                           Index;

  PayloadBootInfo = (EFI_SECURE_PARTITION_BOOT_INFO *)SharedBufAddress;

  if (PayloadBootInfo == NULL) {
    DEBUG ((DEBUG_ERROR, "PayloadBootInfo NULL\n"));
    return NULL;
  }

  if (PayloadBootInfo->Header.Version != BOOT_PAYLOAD_VERSION) {
    DEBUG ((
      DEBUG_ERROR,
      "Boot Information Version Mismatch. Current=0x%x, Expected=0x%x.\n",
      PayloadBootInfo->Header.Version,
      BOOT_PAYLOAD_VERSION
      ));
    return NULL;
  }

  DEBUG ((DEBUG_INFO, "NumSpMemRegions - 0x%x\n", PayloadBootInfo->NumSpMemRegions));
  DEBUG ((DEBUG_INFO, "SpMemBase       - 0x%lx\n", PayloadBootInfo->SpMemBase));
  DEBUG ((DEBUG_INFO, "SpMemLimit      - 0x%lx\n", PayloadBootInfo->SpMemLimit));
  DEBUG ((DEBUG_INFO, "SpImageBase     - 0x%lx\n", PayloadBootInfo->SpImageBase));
  DEBUG ((DEBUG_INFO, "SpStackBase     - 0x%lx\n", PayloadBootInfo->SpStackBase));
  DEBUG ((DEBUG_INFO, "SpHeapBase      - 0x%lx\n", PayloadBootInfo->SpHeapBase));
  DEBUG ((DEBUG_INFO, "SpNsCommBufBase - 0x%lx\n", PayloadBootInfo->SpNsCommBufBase));
  DEBUG ((DEBUG_INFO, "SpSharedBufBase - 0x%lx\n", PayloadBootInfo->SpSharedBufBase));

  DEBUG ((DEBUG_INFO, "SpImageSize     - 0x%x\n", PayloadBootInfo->SpImageSize));
  DEBUG ((DEBUG_INFO, "SpPcpuStackSize - 0x%x\n", PayloadBootInfo->SpPcpuStackSize));
  DEBUG ((DEBUG_INFO, "SpHeapSize      - 0x%x\n", PayloadBootInfo->SpHeapSize));
  DEBUG ((DEBUG_INFO, "SpNsCommBufSize - 0x%x\n", PayloadBootInfo->SpNsCommBufSize));
  DEBUG ((DEBUG_INFO, "SpSharedBufSize - 0x%x\n", PayloadBootInfo->SpSharedBufSize));

  DEBUG ((DEBUG_INFO, "NumCpus         - 0x%x\n", PayloadBootInfo->NumCpus));
  DEBUG ((DEBUG_INFO, "CpuInfo         - 0x%p\n", PayloadBootInfo->CpuInfo));

  PayloadCpuInfo = (EFI_SECURE_PARTITION_CPU_INFO *)PayloadBootInfo->CpuInfo;

  if (PayloadCpuInfo == NULL) {
    DEBUG ((DEBUG_ERROR, "PayloadCpuInfo NULL\n"));
    return NULL;
  }

  for (Index = 0; Index < PayloadBootInfo->NumCpus; Index++) {
    DEBUG ((DEBUG_INFO, "Mpidr           - 0x%lx\n", PayloadCpuInfo[Index].Mpidr));
    DEBUG ((DEBUG_INFO, "LinearId        - 0x%x\n", PayloadCpuInfo[Index].LinearId));
    DEBUG ((DEBUG_INFO, "Flags           - 0x%x\n", PayloadCpuInfo[Index].Flags));
  }

  return PayloadBootInfo;
}

/**
  Get service type.
  When using FF-A ABI, there're ways to request service to StandaloneMm
      - FF-A with MmCommunication protocol.
      - FF-A service with each specification.
   MmCommunication Protocol can use FFA_MSG_SEND_DIRECT_REQ or REQ2,
   Other FF-A services should use FFA_MSG_SEND_DIRECT_REQ2.
   In case of FF-A with MmCommunication protocol via FFA_MSG_SEND_DIRECT_REQ,
   register x3 saves Communication Buffer with gEfiMmCommunication2ProtocolGuid.
   In case of FF-A with MmCommunication protocol via FFA_MSG_SEND_DIRECT_REQ2,
   register x2/x3 save gEfiMmCommunication2ProtocolGuid and
   register x4 saves Communication Buffer with Service Guid.

   Other FF-A services (ServiceTypeMisc) delivers register values according to
   there own service specification.
   That means it doesn't use MmCommunication Buffer with MmCommunication Header
   format.
   (i.e) Tpm service via FF-A or Firmware Update service via FF-A.
   To support latter services by StandaloneMm, it defines SERVICE_TYPE_MISC.
   So that StandaloneMmEntryPointCore.c generates MmCommunication Header
   with delivered register values to dispatch service provided StandaloneMmCore.
   So that service handler can get proper information from delivered register.

   In case of SPM_MM Abi, it only supports MmCommunication service.


  @param[in]      ServiceGuid                   Service Guid

  @retval         ServiceTypeMmCommunication    Mm communication service
  @retval         ServiceTypeMisc               Service via implemented defined
                                                register ABI.
                                                This will generate internal
                                                MmCommunication Header
                                                to dispatch service implemented
                                                in standaloneMm
  @retval         ServiceTypeUnknown            Not supported service.

**/
STATIC
SERVICE_TYPE
EFIAPI
GetServiceType (
  IN EFI_GUID  *ServiceGuid
  )
{
  if (CompareGuid (ServiceGuid, &gEfiMmCommunication2ProtocolGuid)) {
    return ServiceTypeMmCommunication;
  }

  return ServiceTypeMisc;
}

/**
  Perform bounds check for the Ns and Secure Communication buffer.

  NOTE: We do not need to validate the Misc Communication buffer as
  we are initialising that in StandaloneMm.

  @param  [in] CommBufferAddr   Address of the common buffer.

  @retval   EFI_SUCCESS             Success.
  @retval   EFI_ACCESS_DENIED       Access not permitted.
**/
STATIC
EFI_STATUS
ValidateMmCommBufferAddr (
  IN UINTN  CommBufferAddr
  )
{
  UINT64  NsCommBufferEnd;
  UINT64  SCommBufferEnd;
  UINT64  CommBufferEnd;
  UINT64  CommBufferRange;

  NsCommBufferEnd = mNsCommBuffer->PhysicalStart + mNsCommBuffer->PhysicalSize;
  SCommBufferEnd  = mSCommBuffer->PhysicalStart + mSCommBuffer->PhysicalSize;

  if ((CommBufferAddr >= mNsCommBuffer->PhysicalStart) &&
      (CommBufferAddr < NsCommBufferEnd))
  {
    CommBufferEnd = NsCommBufferEnd;
  } else if ((CommBufferAddr >= mSCommBuffer->PhysicalStart) &&
             (CommBufferAddr < SCommBufferEnd))
  {
    CommBufferEnd = SCommBufferEnd;
  } else {
    return EFI_ACCESS_DENIED;
  }

  CommBufferRange = CommBufferEnd - CommBufferAddr;

  if (CommBufferRange < sizeof (EFI_MM_COMMUNICATE_HEADER)) {
    return EFI_ACCESS_DENIED;
  }

  // perform bounds check.
  if (((CommBufferAddr + sizeof (EFI_MM_COMMUNICATE_HEADER) +
        ((EFI_MM_COMMUNICATE_HEADER *)CommBufferAddr)->MessageLength)) >
      CommBufferEnd)
  {
    return EFI_ACCESS_DENIED;
  }

  return EFI_SUCCESS;
}

/**
  Convert EFI_STATUS to MM SPM return code.

  @param [in] Status          edk2 status code.

  @retval ARM_SPM_MM_RET_*    return value correspond to EFI_STATUS.

**/
STATIC
UINTN
EFIAPI
EfiStatusToSpmMmStatus (
  IN EFI_STATUS  Status
  )
{
  switch (Status) {
    case EFI_SUCCESS:
      return ARM_SPM_MM_RET_SUCCESS;
    case EFI_INVALID_PARAMETER:
      return ARM_SPM_MM_RET_INVALID_PARAMS;
    case EFI_ACCESS_DENIED:
      return ARM_SPM_MM_RET_DENIED;
    case EFI_OUT_OF_RESOURCES:
      return ARM_SPM_MM_RET_NO_MEMORY;
    default:
      return ARM_SPM_MM_RET_NOT_SUPPORTED;
  }
}

/**
  Set svc arguments to report initialization status of StandaloneMm.

  @param[in]      CommProtocol              ABI Protocol.
  @param[in]      Status                    Result of initializing StandaloneMm.
  @param[out]     EventCompleteSvcArgs      Args structure.

**/
STATIC
VOID
ReturnInitStatusToSpmc (
  IN COMM_PROTOCOL  CommProtocol,
  IN EFI_STATUS     Status,
  OUT ARM_SVC_ARGS  *EventCompleteSvcArgs
  )
{
  ZeroMem (EventCompleteSvcArgs, sizeof (ARM_SVC_ARGS));

  if (CommProtocol == CommProtocolFfa) {
    if (EFI_ERROR (Status)) {
      EventCompleteSvcArgs->Arg0 = ARM_FID_FFA_ERROR;

      /*
       * In case SvcConduit, this must be zero.
       */
      EventCompleteSvcArgs->Arg1 = 0x00;
      EventCompleteSvcArgs->Arg2 = EfiStatusToFfaStatus (Status);
    } else {
      /*
       * For completion of initialization, It should use FFA_MSG_WAIT.
       * See FF-A specification 5.5 Protocol for completing execution context
       * initialization
       */
      EventCompleteSvcArgs->Arg0 = ARM_FID_FFA_WAIT;
    }
  } else if (CommProtocol == CommProtocolSpmMm) {
    EventCompleteSvcArgs->Arg0 = ARM_FID_SPM_MM_SP_EVENT_COMPLETE;
    EventCompleteSvcArgs->Arg1 = EfiStatusToSpmMmStatus (Status);
  } else {
    /*
     * We don't know what communication abi protocol is using.
     * Set Arg0 as MAX_UINTN to make SPMC know it's error situation.
     */
    EventCompleteSvcArgs->Arg0 = MAX_UINTN;
  }
}

/**
  Set Event Complete arguments to be returned via SVC call.

  @param[in]      CommProtocol              Communication Protocol.
  @param[in]      CommData                  Communication Abi specific data.
  @param[in]      Status                    Result of StandaloneMm service.
  @param[out]     EventCompleteSvcArgs      Args structure.

**/
STATIC
VOID
SetEventCompleteSvcArgs (
  IN COMM_PROTOCOL  CommProtocol,
  IN VOID           *CommData,
  IN EFI_STATUS     Status,
  OUT ARM_SVC_ARGS  *EventCompleteSvcArgs
  )
{
  FFA_MSG_INFO  *FfaMsgInfo;

  ZeroMem (EventCompleteSvcArgs, sizeof (ARM_SVC_ARGS));

  if (CommProtocol == CommProtocolFfa) {
    FfaMsgInfo = CommData;

    if (EFI_ERROR (Status)) {
      EventCompleteSvcArgs->Arg0 = ARM_FID_FFA_ERROR;

      /*
       * StandaloneMm is secure instance. So set as 0x00.
       */
      EventCompleteSvcArgs->Arg1 = 0x00;
      EventCompleteSvcArgs->Arg2 = EfiStatusToFfaStatus (Status);
    } else {
      if (FfaMsgInfo->DirectMsgVersion == DirectMsgV1) {
        EventCompleteSvcArgs->Arg0 = ARM_FID_FFA_MSG_SEND_DIRECT_RESP;
        EventCompleteSvcArgs->Arg3 = ARM_FID_SPM_MM_SP_EVENT_COMPLETE;
      } else {
        EventCompleteSvcArgs->Arg0 = ARM_FID_FFA_MSG_SEND_DIRECT_RESP2;

        if (FfaMsgInfo->ServiceType == ServiceTypeMisc) {
          EventCompleteSvcArgs->Arg4 = mMiscMmCommunicateBuffer->DirectMsgArgs.Arg0;
          EventCompleteSvcArgs->Arg5 = mMiscMmCommunicateBuffer->DirectMsgArgs.Arg1;
          EventCompleteSvcArgs->Arg6 = mMiscMmCommunicateBuffer->DirectMsgArgs.Arg2;
          EventCompleteSvcArgs->Arg7 = mMiscMmCommunicateBuffer->DirectMsgArgs.Arg3;
        }
      }

      /*
       * Swap source & dest partition id.
       */
      EventCompleteSvcArgs->Arg1 = PACK_PARTITION_ID_INFO (
                                     FfaMsgInfo->DestPartId,
                                     FfaMsgInfo->SourcePartId
                                     );
    }
  } else {
    EventCompleteSvcArgs->Arg0 = ARM_FID_SPM_MM_SP_EVENT_COMPLETE;
    EventCompleteSvcArgs->Arg1 = EfiStatusToSpmMmStatus (Status);
  }
}

/**
  Wrap Misc service buffer with MmCommunication Header to
  patch event handler via MmCommunication protocol.

  @param[in]      EventSvcArgs              Passed arguments
  @param[in]      ServiceGuid               Service Guid
  @param[out]     Buffer                    Misc service data
                                            wrapped with MmCommunication Header.

**/
STATIC
VOID
InitializeMiscMmCommunicateBuffer (
  IN ARM_SVC_ARGS                 *EventSvcArgs,
  IN EFI_GUID                     *ServiceGuid,
  OUT MISC_MM_COMMUNICATE_BUFFER  *Buffer
  )
{
  ZeroMem (Buffer, sizeof (MISC_MM_COMMUNICATE_BUFFER));

  Buffer->MessageLength      = sizeof (DIRECT_MSG_ARGS);
  Buffer->DirectMsgArgs.Arg0 = EventSvcArgs->Arg4;
  Buffer->DirectMsgArgs.Arg1 = EventSvcArgs->Arg5;
  Buffer->DirectMsgArgs.Arg2 = EventSvcArgs->Arg6;
  Buffer->DirectMsgArgs.Arg3 = EventSvcArgs->Arg7;
  CopyGuid (&Buffer->HeaderGuid, ServiceGuid);
}

/**
  Convert UUID to EFI_GUID format.
  for example, If there is EFI_GUID named
  "378daedc-f06b-4446-8314-40ab933c87a3",

  EFI_GUID is saved in memory like:
     dc ae 8d 37
     6b f0 46 44
     83 14 40 ab
     93 3c 87 a3

  However, UUID should be saved like:
     37 8d ae dc
     f0 6b 44 46
     83 14 40 ab
     93 3c 87 a3

  FF-A and other software components (i.e. linux-kernel)
  uses below format.

  To patch mm-service properly, the passed uuid should be converted to
  EFI_GUID format.

  @param [in]  Uuid            Uuid
  @param [out] Guid            EFI_GUID

**/
STATIC
VOID
EFIAPI
ConvertUuidToEfiGuid (
  IN  UINT64    *Uuid,
  OUT EFI_GUID  *Guid
  )
{
  UINT32  *Data32;
  UINT16  *Data16;

  Data32    = (UINT32 *)Uuid;
  Data32[0] = SwapBytes32 (Data32[0]);
  Data16    = (UINT16 *)&Data32[1];
  Data16[0] = SwapBytes16 (Data16[0]);
  Data16[1] = SwapBytes16 (Data16[1]);
  CopyGuid (Guid, (EFI_GUID *)Uuid);
}

/**
  A loop to delegate events from SPMC.
  DelegatedEventLoop() calls ArmCallSvc() to exit to SPMC.
  When an event is delegated to StandaloneMm the SPMC returns control
  to StandaloneMm by returning from the SVC call.

  @param  [in] CommProtocol            Abi Protocol.
  @param  [in] CpuDriverEntryPoint    Entry point to handle request.
  @param  [in] EventCompleteSvcArgs   Pointer to the event completion arguments.

**/
STATIC
VOID
EFIAPI
DelegatedEventLoop (
  IN COMM_PROTOCOL                      CommProtocol,
  IN EDKII_PI_MM_CPU_DRIVER_ENTRYPOINT  CpuDriverEntryPoint,
  IN ARM_SVC_ARGS                       *EventCompleteSvcArgs
  )
{
  EFI_STATUS    Status;
  UINT64        Uuid[2];
  VOID          *CommData;
  FFA_MSG_INFO  FfaMsgInfo;
  EFI_GUID      ServiceGuid;
  SERVICE_TYPE  ServiceType;
  UINTN         CommBufferAddr;

  CommData = NULL;

  while (TRUE) {
    // Exit to SPMC.
    ArmCallSvc (EventCompleteSvcArgs);
    // Enter from SPMC.

    DEBUG ((DEBUG_INFO, "Received delegated event\n"));
    DEBUG ((DEBUG_INFO, "X0 :  0x%x\n", (UINT32)EventCompleteSvcArgs->Arg0));
    DEBUG ((DEBUG_INFO, "X1 :  0x%x\n", (UINT32)EventCompleteSvcArgs->Arg1));
    DEBUG ((DEBUG_INFO, "X2 :  0x%x\n", (UINT32)EventCompleteSvcArgs->Arg2));
    DEBUG ((DEBUG_INFO, "X3 :  0x%x\n", (UINT32)EventCompleteSvcArgs->Arg3));
    DEBUG ((DEBUG_INFO, "X4 :  0x%x\n", (UINT32)EventCompleteSvcArgs->Arg4));
    DEBUG ((DEBUG_INFO, "X5 :  0x%x\n", (UINT32)EventCompleteSvcArgs->Arg5));
    DEBUG ((DEBUG_INFO, "X6 :  0x%x\n", (UINT32)EventCompleteSvcArgs->Arg6));
    DEBUG ((DEBUG_INFO, "X7 :  0x%x\n", (UINT32)EventCompleteSvcArgs->Arg7));

    if (CommProtocol == CommProtocolFfa) {
      /*
       * Register Convention for FF-A
       *   Arg0: ARM_FID_FFA_MSG_SEND_DIRECT_REQ/REQ2
       *   Arg1: Sender and Receiver endpoint IDs.
       *   Arg2: Message Flags for ARM_FID_FFA_MSG_SEND_DIRECT_REQ
       *         Low 8 bytes of UUID for ARM_FID_FFA_MSG_SEND_DIRECT_REQ2
       *   Arg3: Implementation Defined for ARM_FID_FFA_MSG_SEND_DIRECT_REQ
       *         High 8 bytes of UUID for ARM_FID_FFA_MSG_SEND_DIRECT_REQ2
       *   Others: Implementation Defined.
       *
       *   See Arm Firmware Framework for Arm A-Profile for detail.
       */
      FfaMsgInfo.SourcePartId = GET_SOURCE_PARTITION_ID (EventCompleteSvcArgs->Arg1);
      FfaMsgInfo.DestPartId   = GET_DEST_PARTITION_ID (EventCompleteSvcArgs->Arg1);
      CommData                = &FfaMsgInfo;

      if (EventCompleteSvcArgs->Arg0 == ARM_FID_FFA_MSG_SEND_DIRECT_REQ) {
        FfaMsgInfo.DirectMsgVersion = DirectMsgV1;
        ServiceType                 = ServiceTypeMmCommunication;
      } else if (EventCompleteSvcArgs->Arg0 == ARM_FID_FFA_MSG_SEND_DIRECT_REQ2) {
        FfaMsgInfo.DirectMsgVersion = DirectMsgV2;
        Uuid[0]                     = EventCompleteSvcArgs->Arg2;
        Uuid[1]                     = EventCompleteSvcArgs->Arg3;
        ConvertUuidToEfiGuid (Uuid, &ServiceGuid);
        ServiceType = GetServiceType (&ServiceGuid);
      } else {
        Status = EFI_INVALID_PARAMETER;
        DEBUG ((
          DEBUG_ERROR,
          "Error: Unrecognized FF-A Id: 0x%x\n",
          EventCompleteSvcArgs->Arg0
          ));
        goto ExitHandler;
      }

      FfaMsgInfo.ServiceType = ServiceType;

      if (ServiceType == ServiceTypeMmCommunication) {
        if (FfaMsgInfo.DirectMsgVersion == DirectMsgV1) {
          CommBufferAddr = EventCompleteSvcArgs->Arg3;
        } else {
          CommBufferAddr = EventCompleteSvcArgs->Arg4;
        }
      } else if (ServiceType == ServiceTypeMisc) {
        /*
         * In case of Misc service, generate mm communication header
         * to dispatch service via StandaloneMmCore.
         */
        InitializeMiscMmCommunicateBuffer (
          EventCompleteSvcArgs,
          &ServiceGuid,
          mMiscMmCommunicateBuffer
          );
        CommBufferAddr = (UINTN)mMiscMmCommunicateBuffer;
      } else {
        Status = EFI_INVALID_PARAMETER;
        DEBUG ((DEBUG_ERROR, "Error: Invalid FF-A Service...\n"));
        goto ExitHandler;
      }
    } else if (CommProtocol == CommProtocolSpmMm) {
      /*
       * Register Convention for SPM_MM
       *   Arg0: ARM_SMC_ID_MM_COMMUNICATE
       *   Arg1: Communication Buffer
       *   Arg2: Size of Communication Buffer
       *   Arg3: Cpu number where StandaloneMm running on.
       *
       *   See tf-a/services/std_svc/spm/spm_mm/spm_mm_main.c
       */
      if (EventCompleteSvcArgs->Arg0 != ARM_SMC_ID_MM_COMMUNICATE) {
        Status = EFI_INVALID_PARAMETER;
        DEBUG ((
          DEBUG_ERROR,
          "Error: Unrecognized SPM_MM Id: 0x%x\n",
          EventCompleteSvcArgs->Arg0
          ));
        goto ExitHandler;
      }

      CommBufferAddr = EventCompleteSvcArgs->Arg1;
      ServiceType    = ServiceTypeMmCommunication;
    }

    if (ServiceType == ServiceTypeMmCommunication) {
      Status = ValidateMmCommBufferAddr (CommBufferAddr);
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "Error: Failed to validate Communication Buffer address(0x%x)...\n",
          CommBufferAddr
          ));
        goto ExitHandler;
      }
    }

    Status = CpuDriverEntryPoint ((UINTN)ServiceType, CommBufferAddr);
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "Error: Failed delegated event 0x%x, Status 0x%x\n",
        CommBufferAddr,
        Status
        ));
    }

ExitHandler:
    SetEventCompleteSvcArgs (
      CommProtocol,
      CommData,
      Status,
      EventCompleteSvcArgs
      );
  } // while
}

/**
  The entry point of Standalone MM Foundation.

  @param  [in]  SharedBufAddress  Pointer to the Buffer between SPM and SP.
  @param  [in]  SharedBufSize     Size of the shared buffer.
  @param  [in]  cookie1           Cookie 1
  @param  [in]  cookie2           Cookie 2

**/
VOID
EFIAPI
_ModuleEntryPoint (
  IN VOID    *SharedBufAddress,
  IN UINT64  SharedBufSize,
  IN UINT64  cookie1,
  IN UINT64  cookie2
  )
{
  PE_COFF_LOADER_IMAGE_CONTEXT        ImageContext;
  EFI_SECURE_PARTITION_BOOT_INFO      *PayloadBootInfo;
  ARM_SVC_ARGS                        EventCompleteSvcArgs;
  EFI_STATUS                          Status;
  UINT32                              SectionHeaderOffset;
  UINT16                              NumberOfSections;
  COMM_PROTOCOL                       CommProtocol;
  VOID                                *HobStart;
  VOID                                *TeData;
  UINTN                               TeDataSize;
  EFI_PHYSICAL_ADDRESS                ImageBase;
  EDKII_PI_MM_CPU_DRIVER_EP_PROTOCOL  *PiMmCpuDriverEpProtocol;
  EDKII_PI_MM_CPU_DRIVER_ENTRYPOINT   CpuDriverEntryPoint;
  EFI_HOB_GUID_TYPE                   *GuidHob;
  EFI_CONFIGURATION_TABLE             *ConfigurationTable;
  UINTN                               Idx;
  EFI_MMRAM_HOB_DESCRIPTOR_BLOCK      *MmramRangesHob;

  CpuDriverEntryPoint = NULL;

  Status = GetCommProtocol (&CommProtocol);
  if (EFI_ERROR (Status)) {
    goto finish;
  }

  PayloadBootInfo = GetAndPrintBootinformation (SharedBufAddress);
  if (PayloadBootInfo == NULL) {
    Status = EFI_UNSUPPORTED;
    goto finish;
  }

  // Locate PE/COFF File information for the Standalone MM core module
  Status = LocateStandaloneMmCorePeCoffData (
             (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)PayloadBootInfo->SpImageBase,
             &TeData,
             &TeDataSize
             );

  if (EFI_ERROR (Status)) {
    goto finish;
  }

  // Obtain the PE/COFF Section information for the Standalone MM core module
  Status = GetStandaloneMmCorePeCoffSections (
             TeData,
             &ImageContext,
             &ImageBase,
             &SectionHeaderOffset,
             &NumberOfSections
             );

  if (EFI_ERROR (Status)) {
    goto finish;
  }

  //
  // ImageBase may deviate from ImageContext.ImageAddress if we are dealing
  // with a TE image, in which case the latter points to the actual offset
  // of the image, whereas ImageBase refers to the address where the image
  // would start if the stripped PE headers were still in place. In either
  // case, we need to fix up ImageBase so it refers to the actual current
  // load address.
  //
  ImageBase += (UINTN)TeData - ImageContext.ImageAddress;

  // Update the memory access permissions of individual sections in the
  // Standalone MM core module
  Status = UpdateMmFoundationPeCoffPermissions (
             &ImageContext,
             ImageBase,
             SectionHeaderOffset,
             NumberOfSections,
             ArmSetMemoryRegionNoExec,
             ArmSetMemoryRegionReadOnly,
             ArmClearMemoryRegionReadOnly
             );

  if (EFI_ERROR (Status)) {
    goto finish;
  }

  if (ImageContext.ImageAddress != (UINTN)TeData) {
    ImageContext.ImageAddress = (UINTN)TeData;
    ArmSetMemoryRegionNoExec (ImageBase, SIZE_4KB);
    ArmClearMemoryRegionReadOnly (ImageBase, SIZE_4KB);

    Status = PeCoffLoaderRelocateImage (&ImageContext);
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Create Hoblist based upon boot information passed by privileged software
  //
  HobStart = CreateHobListFromBootInfo (PayloadBootInfo);

  //
  // Call the MM Core entry point
  //
  ProcessModuleEntryPointList (HobStart);

  // ProcessModuleEntryPointList() copies the HOB List passed
  // by TF-A, i.e. HobStart, in the ConfigurationTable[].
  // Therefore, find the HobList in the ConfigurationTable[] by
  // searching for the gEfiHobListGuid.
  // Also update the gHobList to point to the HobList in the
  // ConfigurationTable[] as the HobList passed by TF-A can
  // be overwritten by StMM after StMM Core is initialised, i.e.
  // after the MM Core entry point is called.
  Status             = EFI_NOT_FOUND;
  ConfigurationTable = gMmCoreMmst.MmConfigurationTable;
  for (Idx = 0; Idx < gMmCoreMmst.NumberOfTableEntries; Idx++) {
    if (CompareGuid (&gEfiHobListGuid, &ConfigurationTable[Idx].VendorGuid)) {
      Status = EFI_SUCCESS;
      break;
    }
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error: Hoblist not found in MmConfigurationTable\n"));
    goto finish;
  }

  gHobList = ConfigurationTable[Idx].VendorTable;

  // Find the descriptor that contains the whereabouts of the buffer for
  // communication with the Normal world.
  GuidHob = GetNextGuidHob (&gEfiStandaloneMmNonSecureBufferGuid, gHobList);
  if (GuidHob == NULL) {
    Status = EFI_NOT_FOUND;
    DEBUG ((DEBUG_ERROR, "Error: No NsCommBuffer hob ...\n"));
    goto finish;
  }

  mNsCommBuffer = GET_GUID_HOB_DATA (GuidHob);
  if (mNsCommBuffer == NULL) {
    Status = EFI_NOT_FOUND;
    DEBUG ((DEBUG_ERROR, "Error: No NsCommBuffer hob data...\n"));
    goto finish;
  }

  //
  // The base and size of buffer shared with
  // privileged Secure world software is in PeiMmramMemoryReservedGuid Hob.
  //
  GuidHob = GetNextGuidHob (&gEfiMmPeiMmramMemoryReserveGuid, gHobList);
  if (GuidHob == NULL) {
    Status = EFI_NOT_FOUND;
    DEBUG ((DEBUG_ERROR, "Error: No PeiMmramMemoryReserved hob ...\n"));
    goto finish;
  }

  MmramRangesHob = GET_GUID_HOB_DATA (GuidHob);
  if ((MmramRangesHob == NULL) ||
      (MmramRangesHob->NumberOfMmReservedRegions < MMRAM_DESC_MIN_COUNT))
  {
    Status = EFI_NOT_FOUND;
    DEBUG ((DEBUG_ERROR, "Error: Failed to get shared comm buffer ...\n"));
    goto finish;
  }

  mSCommBuffer = &MmramRangesHob->Descriptor[MMRAM_DESC_IDX_SECURE_SHARED_BUFFER];

  //
  // Find out cpu driver entry point used in DelegatedEventLoop
  // to handle MMI request.
  //
  Status = gMmCoreMmst.MmLocateProtocol (
                         &gEdkiiPiMmCpuDriverEpProtocolGuid,
                         NULL,
                         (VOID **)&PiMmCpuDriverEpProtocol
                         );
  if (EFI_ERROR (Status)) {
    goto finish;
  }

  CpuDriverEntryPoint = PiMmCpuDriverEpProtocol->PiMmCpuDriverEntryPoint;

  DEBUG ((
    DEBUG_INFO,
    "Shared Cpu Driver EP %p\n",
    CpuDriverEntryPoint
    ));

  if (CommProtocol == CommProtocolFfa) {
    Status = gMmCoreMmst.MmAllocatePool (
                           EfiRuntimeServicesData,
                           sizeof (MISC_MM_COMMUNICATE_BUFFER),
                           (VOID **)&mMiscMmCommunicateBuffer
                           );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "Error: Failed to allocate misc mm communication buffer...\n"
        ));
      goto finish;
    }
  }

finish:
  ReturnInitStatusToSpmc (CommProtocol, Status, &EventCompleteSvcArgs);

  // Call DelegateEventLoop(), this function never returns.
  DelegatedEventLoop (CommProtocol, CpuDriverEntryPoint, &EventCompleteSvcArgs);
}
