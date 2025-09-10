/** @file

  Copyright (c) 2019, Linaro Limited. All rights reserved.
  Copyright (c) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

// Number of Virtual Memory Map Descriptors
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS  5

// MU_CHANGE START

/**
  Checks if the platform requires a special initial EFI memory region.

  @param[out]  EfiMemoryBase  The custom memory base, will be unchanged if FALSE is returned.
  @param[out]  EfiMemorySize  The custom memory size, will be unchanged if FALSE is returned.

  @retval   TRUE    A custom memory region was set.
  @retval   FALSE   A custom memory region was not set.
**/
BOOLEAN
EFIAPI
ArmPlatformGetPeiMemory (
  OUT UINTN   *EfiMemoryBase,
  OUT UINT32  *EfiMemorySize
  )
{
  return FALSE;
}

// MU_CHANGE END

RETURN_STATUS
EFIAPI
SbsaQemuLibConstructor (
  VOID
  )
{
  return RETURN_SUCCESS;
}

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU
  on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR
                                    describing a Physical-to-Virtual Memory
                                    mapping. This array must be ended by a
                                    zero-filled entry. The allocated memory
                                    will not be freed.

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  OUT ARM_MEMORY_REGION_DESCRIPTOR  **VirtualMemoryMap
  )
{
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  ASSERT (VirtualMemoryMap != NULL);

  VirtualMemoryTable = AllocatePool (
                         sizeof (ARM_MEMORY_REGION_DESCRIPTOR) *
                         MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS
                         );

  if (VirtualMemoryTable == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Error: Failed AllocatePool()\n", __FUNCTION__));
    return;
  }

  // System DRAM
  VirtualMemoryTable[0].PhysicalBase = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[0].VirtualBase  = VirtualMemoryTable[0].PhysicalBase;
  VirtualMemoryTable[0].Length       = PcdGet64 (PcdSystemMemorySize);
  VirtualMemoryTable[0].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  DEBUG ((
    DEBUG_INFO,
    "%a: Dumping System DRAM Memory Map:\n"
    "\tPhysicalBase: 0x%lX\n"
    "\tVirtualBase: 0x%lX\n"
    "\tLength: 0x%lX\n",
    __FUNCTION__,
    VirtualMemoryTable[0].PhysicalBase,
    VirtualMemoryTable[0].VirtualBase,
    VirtualMemoryTable[0].Length
    ));

  // Peripheral space before DRAM
  VirtualMemoryTable[1].PhysicalBase = 0x0;
  VirtualMemoryTable[1].VirtualBase  = 0x0;
  VirtualMemoryTable[1].Length       = VirtualMemoryTable[0].PhysicalBase;
  VirtualMemoryTable[1].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // Remap the FD region as normal executable memory
  VirtualMemoryTable[2].PhysicalBase = PcdGet64 (PcdFdBaseAddress);
  VirtualMemoryTable[2].VirtualBase  = VirtualMemoryTable[2].PhysicalBase;
  VirtualMemoryTable[2].Length       = FixedPcdGet32 (PcdFdSize);
  VirtualMemoryTable[2].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // MM Memory Space
  VirtualMemoryTable[3].PhysicalBase = PcdGet64 (PcdMmBufferBase);
  VirtualMemoryTable[3].VirtualBase  = PcdGet64 (PcdMmBufferBase);
  VirtualMemoryTable[3].Length       = PcdGet64 (PcdMmBufferSize);
  VirtualMemoryTable[3].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // End of Table
  ZeroMem (&VirtualMemoryTable[4], sizeof (ARM_MEMORY_REGION_DESCRIPTOR));

  *VirtualMemoryMap = VirtualMemoryTable;
}
