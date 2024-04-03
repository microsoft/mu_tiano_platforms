/** @file
  The CPU specific programming for Standalone MM environment.

  Copyright (c) 2010 - 2015, Intel Corporation. All rights reserved.<BR>
  Copyright (c) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiMm.h>
#include <Library/SmmCpuFeaturesLib.h>
#include <Library/MmServicesTableLib.h>

/**
  Hook point in normal execution mode that allows the one CPU that was elected
  as monarch during System Management Mode initialization to perform additional
  initialization actions immediately after all of the CPUs have processed their
  first SMI and called SmmCpuFeaturesInitializeProcessor() relocating SMBASE
  into a buffer in SMRAM and called SmmCpuFeaturesHookReturnFromSmm().
**/
VOID
EFIAPI
SmmCpuFeaturesSmmRelocationComplete (
  VOID
  )
{
  // Do nothing for Standalone MM instance.
}

/**
  Processor specific hook point each time a CPU exits System Management Mode.

  @param[in] CpuIndex  The index of the CPU that is exiting SMM.  The value
                       must be between 0 and the NumberOfCpus field in the
                       System Management System Table (SMST).
**/
VOID
EFIAPI
SmmCpuFeaturesRendezvousExit (
  IN UINTN  CpuIndex
  )
{
  // Do nothing for Standalone MM instance.
}
