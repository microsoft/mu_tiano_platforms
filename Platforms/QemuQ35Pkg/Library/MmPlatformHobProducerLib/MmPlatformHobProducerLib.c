/** @file
  Null instance of MM Platform HOB Producer Library Class.

  CreateMmPlatformHob() function is called by StandaloneMm IPL to create all
  Platform specific HOBs that required by Standalone MM environment.

  Copyright (c) 2024, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <PiPei.h>
#include <AdvancedLoggerInternal.h>

#include <Guid/MmCommonRegion.h>
#include <Guid/MmramMemoryReserve.h>

#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MmPlatformHobProducerLib.h>

#define NUMBER_OF_HOB_RESOURCE_DESCRIPTOR  3

/**
  Copies a data buffer to a newly-built HOB for GUID HOB

  This function builds a customized HOB tagged with a GUID for identification, copies the
  input data to the HOB data field and returns the start address of the GUID HOB data.
  If new HOB buffer is NULL or the GUID HOB could not found, then ASSERT().

  @param[in]       HobBuffer            The pointer of HOB buffer.
  @param[in, out]  HobBufferSize        The available size of the HOB buffer when as input.
                                        The used size of when as output.
  @param[in]       Guid                 The GUID of the GUID type HOB.

  @retval          EFI_SUCCESS            The request succeeded.
  @retval          EFI_INVALID_PARAMETER  HobBufferSize is NULL, or HobBuffer is not NULL while the size is non-zero.
  @retval
**/
EFI_STATUS
MmPlatformHobCopyGuidHob (
  IN UINT8      *HobBuffer,
  IN OUT UINTN  *HobBufferSize,
  IN EFI_GUID   *Guid
  )
{
  EFI_HOB_GENERIC_HEADER  *GuidHob;
  UINTN                   UsedSize;
  EFI_STATUS              Status;

  if (HobBufferSize == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if ((HobBuffer == NULL) && (*HobBufferSize != 0)) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  UsedSize = 0;
  GuidHob  = GetFirstGuidHob (Guid);
  ASSERT (GuidHob != NULL);

  Status = EFI_SUCCESS;

  while (GuidHob != NULL) {
    if (*HobBufferSize >= UsedSize + GuidHob->HobLength) {
      if (HobBuffer != NULL) {
        CopyMem (HobBuffer + UsedSize, GuidHob, GuidHob->HobLength);
      }
    } else {
      Status = EFI_BUFFER_TOO_SMALL;
    }

    UsedSize += GuidHob->HobLength;

    GuidHob = GetNextGuidHob (Guid, GET_NEXT_HOB (GuidHob));
  }

  *HobBufferSize = UsedSize;

Done:
  return Status;
}

/**
  Create the platform specific HOBs needed by the Standalone MM environment.

  The following HOBs are created by StandaloneMm IPL common logic.
  Hence they should NOT be created by this function:
  * Single EFI_HOB_TYPE_FV to describe the Firmware Volume where MM Core resides.
  * Single GUIDed (gEfiSmmSmramMemoryGuid) HOB to describe the MM regions.
  * Single EFI_HOB_MEMORY_ALLOCATION_MODULE to describe the MM region used by MM Core.
  * Multiple EFI_HOB_RESOURCE_DESCRIPTOR to describe the non-MM regions and their access permissions.
    Note: All accessible non-MM regions should be described by EFI_HOB_RESOURCE_DESCRIPTOR HOBs.
  * Single GUIDed (gMmCommBufferHobGuid) HOB to identify MM Communication buffer in non-MM region.
  * Multiple GUIDed (gSmmBaseHobGuid) HOB to describe the SMM base address of each processor.
  * Multiple GUIDed (gMpInformation2HobGuid) HOB to describe the MP information.
  * Single GUIDed (gMmCpuSyncConfigHobGuid) HOB to describe how BSP synchronizes with APs in x86 SMM.
  * Single GUIDed (gMmAcpiS3EnableHobGuid) HOB to describe the ACPI S3 enable status.
  * Single GUIDed (gEfiAcpiVariableGuid) HOB to identify the S3 data root region in x86.
  * Single GUIDed (gMmProfileDataHobGuid) HOB to describe the MM profile data region.
  * Single GUIDed (gMmStatusCodeUseSerialHobGuid) HOB to describe the status code use serial port.

  @param[in]      Buffer            The free buffer to be used for HOB creation.
  @param[in, out] BufferSize        The buffer size.
                                    On return, the expected/used size.

  @retval RETURN_INVALID_PARAMETER  BufferSize is NULL.
  @retval RETURN_INVALID_PARAMETER  Buffer is NULL and BufferSize is not 0.
  @retval RETURN_BUFFER_TOO_SMALL   The buffer is too small for HOB creation.
                                    BufferSize is updated to indicate the expected buffer size.
                                    When the input BufferSize is bigger than the expected buffer size,
                                    the BufferSize value will be changed to the used buffer size.
  @retval RETURN_SUCCESS            The HOB list is created successfully.

**/
EFI_STATUS
EFIAPI
CreateMmPlatformHob (
  IN      VOID   *Buffer,
  IN OUT  UINTN  *BufferSize
  )
{
  EFI_HOB_GUID_TYPE  *AdvLoggerGuidHob;
  EFI_HOB_GUID_TYPE  *SupvCommGuidHob;
  UINTN              Size;
  UINTN              UnblockSize;
  UINTN              NumberOfHobResourceDescriptor;
  EFI_HOB_RESOURCE_DESCRIPTOR  Hob;
  EFI_HOB_GUID_TYPE               *MmramRangesHob;
  EFI_MMRAM_HOB_DESCRIPTOR_BLOCK  *MmramRangesHobData;
  EFI_MMRAM_DESCRIPTOR            *MmramRanges;
  UINTN                           MmramRangeCount;
  UINTN                           Index;
  EFI_STATUS                      Status;

  if (BufferSize == NULL) {
    return RETURN_INVALID_PARAMETER;
  }

  if ((*BufferSize != 0) && (Buffer == NULL)) {
    return RETURN_INVALID_PARAMETER;
  }

  Size = 0;

  // Account for the Platform Info HOB
  AdvLoggerGuidHob = GetFirstGuidHob (&gAdvancedLoggerHobGuid);
  Size   += GET_HOB_LENGTH (AdvLoggerGuidHob);

  // Account for resource descriptor HOBs for MM region
  Size += sizeof (EFI_HOB_RESOURCE_DESCRIPTOR) * NUMBER_OF_HOB_RESOURCE_DESCRIPTOR;

  // Account for MMRAM regions, in the format of resource descriptor HOBs

  MmramRangesHob = GetFirstGuidHob (&gEfiMmPeiMmramMemoryReserveGuid);
  if (MmramRangesHob == NULL) {
    MmramRangesHob = GetFirstGuidHob (&gEfiSmmSmramMemoryGuid);
    if (MmramRangesHob == NULL) {
      DEBUG ((DEBUG_ERROR, "Failed to find MMRAM ranges HOB\n"));
      ASSERT (FALSE);
      return RETURN_NOT_FOUND;
    }
  }

  MmramRangesHobData = GET_GUID_HOB_DATA (MmramRangesHob);
  ASSERT (MmramRangesHobData != NULL);
  MmramRanges     = MmramRangesHobData->Descriptor;
  MmramRangeCount = (UINTN)MmramRangesHobData->NumberOfMmReservedRegions;
  ASSERT (MmramRanges);
  ASSERT (MmramRangeCount);

  for (Index = 0; Index < MmramRangeCount; Index++) {
    Size += sizeof (EFI_HOB_RESOURCE_DESCRIPTOR);
  }

  // Next we need to tunnel through the gMmCommonRegionHobGuid GUID HOB to activate
  // supervisor comm buffer.
  SupvCommGuidHob = GetFirstGuidHob (&gMmCommonRegionHobGuid);
  Size   += GET_HOB_LENGTH (SupvCommGuidHob);

  // Then coalesce all the supervisor unblock requests.
  UnblockSize = 0;
  Status = MmPlatformHobCopyGuidHob (NULL, &UnblockSize, &gMmSupvUnblockRegionHobGuid);
  if (Status != RETURN_BUFFER_TOO_SMALL) {
    // This cannot be right...
    return RETURN_DEVICE_ERROR;
  } else {
    Size += UnblockSize;
  }

  if (Size > *BufferSize) {
    *BufferSize = Size;
    return RETURN_BUFFER_TOO_SMALL;
  }

  *BufferSize = Size;

  Size                          = 0;
  NumberOfHobResourceDescriptor = 0;

  // Duplicate the Platform Info HOB
  CopyMem (Buffer, AdvLoggerGuidHob, GET_HOB_LENGTH (AdvLoggerGuidHob));
  Size += GET_HOB_LENGTH (AdvLoggerGuidHob);

  // Duplicate gMmCommonRegionHobGuid GUID HOB
  CopyMem ((UINT8 *)Buffer + Size, SupvCommGuidHob, GET_HOB_LENGTH (SupvCommGuidHob));
  Size += GET_HOB_LENGTH (SupvCommGuidHob);

  // Create resource descriptor HOBs for MM region
  // APIC
  Hob.Header.HobType    = EFI_HOB_TYPE_RESOURCE_DESCRIPTOR;
  Hob.Header.HobLength  = (UINT16)sizeof (EFI_HOB_RESOURCE_DESCRIPTOR);
  Hob.ResourceType      = EFI_RESOURCE_MEMORY_MAPPED_IO;
  Hob.ResourceAttribute = EFI_RESOURCE_ATTRIBUTE_PRESENT     |
                          EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
                          EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
                          EFI_RESOURCE_ATTRIBUTE_TESTED;
  Hob.PhysicalStart  = PcdGet32 (PcdCpuLocalApicBaseAddress);
  Hob.ResourceLength = SIZE_1MB;
  ZeroMem (&(Hob.Owner), sizeof (EFI_GUID));
  CopyMem ((UINT8 *)Buffer + Size, &Hob, sizeof (EFI_HOB_RESOURCE_DESCRIPTOR));
  Size += sizeof (EFI_HOB_RESOURCE_DESCRIPTOR);
  NumberOfHobResourceDescriptor++;

  // PCIe
  Hob.Header.HobType    = EFI_HOB_TYPE_RESOURCE_DESCRIPTOR;
  Hob.Header.HobLength  = (UINT16)sizeof (EFI_HOB_RESOURCE_DESCRIPTOR);
  Hob.ResourceType      = EFI_RESOURCE_MEMORY_MAPPED_IO;
  Hob.ResourceAttribute = EFI_RESOURCE_ATTRIBUTE_PRESENT     |
                          EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
                          EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
                          EFI_RESOURCE_ATTRIBUTE_TESTED;
  Hob.PhysicalStart  = FixedPcdGet64 (PcdPciExpressBaseAddress);
  Hob.ResourceLength = SIZE_256MB;
  ZeroMem (&(Hob.Owner), sizeof (EFI_GUID));
  CopyMem ((UINT8 *)Buffer + Size, &Hob, sizeof (EFI_HOB_RESOURCE_DESCRIPTOR));
  Size += sizeof (EFI_HOB_RESOURCE_DESCRIPTOR);
  NumberOfHobResourceDescriptor++;

  Hob.Header.HobType    = EFI_HOB_TYPE_RESOURCE_DESCRIPTOR;
  Hob.Header.HobLength  = (UINT16)sizeof (EFI_HOB_RESOURCE_DESCRIPTOR);
  Hob.ResourceType      = EFI_RESOURCE_MEMORY_MAPPED_IO;
  Hob.ResourceAttribute = EFI_RESOURCE_ATTRIBUTE_PRESENT     |
                          EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
                          EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
                          EFI_RESOURCE_ATTRIBUTE_TESTED;
  Hob.PhysicalStart  = PcdGet32 (PcdOvmfFdBaseAddress);
  Hob.ResourceLength = PcdGet32 (PcdOvmfFirmwareFdSize);
  ZeroMem (&(Hob.Owner), sizeof (EFI_GUID));
  CopyMem ((UINT8 *)Buffer + Size, &Hob, sizeof (EFI_HOB_RESOURCE_DESCRIPTOR));
  Size += sizeof (EFI_HOB_RESOURCE_DESCRIPTOR);
  NumberOfHobResourceDescriptor++;

  ASSERT (NumberOfHobResourceDescriptor == NUMBER_OF_HOB_RESOURCE_DESCRIPTOR);

  // Account for MMRAM regions
  for (Index = 0; Index < MmramRangeCount; Index++) {
    // Create resource descriptor HOBs for MM region
    Hob.Header.HobType    = EFI_HOB_TYPE_RESOURCE_DESCRIPTOR;
    Hob.Header.HobLength  = (UINT16)sizeof (EFI_HOB_RESOURCE_DESCRIPTOR);
    Hob.ResourceType      = EFI_RESOURCE_SYSTEM_MEMORY;
    Hob.ResourceAttribute = EFI_RESOURCE_ATTRIBUTE_PRESENT     |
                            EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
                            EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
                            EFI_RESOURCE_ATTRIBUTE_TESTED;
    Hob.PhysicalStart  = MmramRanges[Index].CpuStart;
    Hob.ResourceLength = MmramRanges[Index].PhysicalSize;
    ZeroMem (&(Hob.Owner), sizeof (EFI_GUID));
    CopyMem ((UINT8 *)Buffer + Size, &Hob, sizeof (EFI_HOB_RESOURCE_DESCRIPTOR));
    Size += sizeof (EFI_HOB_RESOURCE_DESCRIPTOR);
    NumberOfHobResourceDescriptor++;
  }

  // Finally copy over all the unblock requests
  UnblockSize = *BufferSize - Size;
  Status = MmPlatformHobCopyGuidHob ((UINT8 *)Buffer + Size, &UnblockSize, &gMmSupvUnblockRegionHobGuid);
  if (EFI_ERROR (Status)) {
    // This cannot be right...
    return Status;
  }

  return EFI_SUCCESS;
}
