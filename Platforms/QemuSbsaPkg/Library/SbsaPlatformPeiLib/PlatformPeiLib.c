/** @file

  Copyright (c) 2011-2014, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Guid/SmmuConfig.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <PiPei.h>

#define SBSAQEMU_ACPI_HEADER(Signature, Type, Revision)                                        \
  {                                                                                            \
    Signature,                                    /* UINT32  Signature */                      \
    sizeof (Type),                                /* UINT32  Length */                         \
    Revision,                                     /* UINT8   Revision */                       \
    0,                                            /* UINT8   Checksum */                       \
    { 'L', 'I', 'N', 'A', 'R', 'O' },             /* UINT8   OemId[6] */                       \
    FixedPcdGet64 (PcdAcpiDefaultOemTableId),     /* UINT64  OemTableId */                     \
    FixedPcdGet32 (PcdAcpiDefaultOemRevision),    /* UINT32  OemRevision */                    \
    FixedPcdGet32 (PcdAcpiDefaultCreatorId),      /* UINT32  CreatorId */                      \
    FixedPcdGet32 (PcdAcpiDefaultCreatorRevision) /* UINT32  CreatorRevision */                \
  }

EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  )
{

  BuildFvHob(PcdGet64(PcdFvBaseAddress), PcdGet32(PcdFvSize));

  // Create SMMU Config Hob struct. Same as IORT we want to publish as it is platform dependent.
  SMMU_CONFIG SmmuConfig = {

    // Initialize IORT Table Header
    .Config.Iort = {
      SBSAQEMU_ACPI_HEADER(EFI_ACPI_6_0_IO_REMAPPING_TABLE_SIGNATURE,
        PLATFORM_IO_REMAPPING_STRUCTURE,
        EFI_ACPI_IO_REMAPPING_TABLE_REVISION_00),
      3,
      sizeof(EFI_ACPI_6_0_IO_REMAPPING_TABLE), // NodeOffset
      0
    },

    // Initialize SMMU3 Structure
    .Config.SmmuNode = {
      {
        {
          EFI_ACPI_IORT_TYPE_SMMUv3,
          sizeof(PLATFORM_ACPI_6_0_IO_REMAPPING_SMMU3_NODE),
          2, // Revision
          0, // Reserved
          1, // NumIdMapping
          OFFSET_OF(PLATFORM_ACPI_6_0_IO_REMAPPING_SMMU3_NODE,
            SmmuIdMap) // IdReference
        },
        0x60050000, // Base address
        EFI_ACPI_IORT_SMMUv3_FLAG_COHAC_OVERRIDE, // Flags
        0, // Reserved
        0, // VATOS address
        EFI_ACPI_IORT_SMMUv3_MODEL_GENERIC, // SMMUv3 Model
        74, // Event
        75, // Pri
        77, // Gerror
        76, // Sync
        0,  // Proximity domain
        1   // DevIDMappingIndex
      },
      {
        0x0000, // InputBase
        0xffff, // NumIds
        0x0000, // OutputBase
        OFFSET_OF(PLATFORM_IO_REMAPPING_STRUCTURE, ItsNode), // OutputReference
        0 // Flags
      }
    },

    // Initialize RC Node
    .Config.RcNode = {
      {
        {
          EFI_ACPI_IORT_TYPE_ROOT_COMPLEX, // Type
          sizeof(PLATFORM_ACPI_6_0_IO_REMAPPING_RC_NODE), // Length
          0, // Revision
          0, // Reserved
          1, // NumIdMappings
          OFFSET_OF(PLATFORM_ACPI_6_0_IO_REMAPPING_RC_NODE,
            RcIdMap) // IdReference
        },
        1, // CacheCoherent
        0, // AllocationHints
        0, // Reserved
        1, // MemoryAccessFlags
        EFI_ACPI_IORT_ROOT_COMPLEX_ATS_UNSUPPORTED, // AtsAttribute
        0x0,  // PciSegmentNumber
        // 0, //MemoryAddressSizeLimit
      },
      {
        0x0000, // InputBase
        0xffff, // NumIds
        0x0000, // OutputBase
        OFFSET_OF(PLATFORM_IO_REMAPPING_STRUCTURE,
          SmmuNode), // OutputReference
        0, // Flags
      }
    },

    // Initialize ITS Node
    .Config.ItsNode = {
      // EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE
      {
        // EFI_ACPI_6_0_IO_REMAPPING_NODE
        {
          EFI_ACPI_IORT_TYPE_ITS_GROUP, // Type
          sizeof(PLATFORM_ACPI_6_0_IO_REMAPPING_ITS_NODE), // Length
          0, // Revision
          0, // Identifier
          0, // NumIdMappings
          0, // IdReference
        },
        1, // ITS count
      },
      0 // GIC ITS Identifiers
    }
  };

  BuildGuidDataHob( & gSmmuConfigHobGuid, & SmmuConfig, sizeof(SMMU_CONFIG));

  DEBUG((DEBUG_INFO, "Configured SmmuConfig Hob.\n"));

  return EFI_SUCCESS;
}
