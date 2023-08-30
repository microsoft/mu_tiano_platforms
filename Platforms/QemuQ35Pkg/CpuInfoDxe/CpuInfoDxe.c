/** @file CpuInfoDxe.c
    DXE driver that prints CPU branding information

    Copyright (c) Microsoft Corporation. All rights reserved.
    SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/CpuLib.h>
#include <Register/Intel/Cpuid.h>

#define MAX_MESSAGE_LENGTH  64

STATIC CHAR8  mMessage[MAX_MESSAGE_LENGTH];

/**
  This function returns a CHAR8* that contains the CPU brand name.
**/
CHAR8 *
GetCpuBrandString (
  VOID
  )
{
  STATIC_ASSERT (
    MAX_MESSAGE_LENGTH >= ((3 * 4 * 4) + 1),
    "Max message length is too small."
    );

  // Update CPUID data (ASCII chars) directly into mMessage buffer
  AsmCpuid (CPUID_BRAND_STRING1, (UINT32 *)&(mMessage[0x00]), (UINT32 *)&(mMessage[0x04]), (UINT32 *)&(mMessage[0x08]), (UINT32 *)&(mMessage[0x0C]));
  AsmCpuid (CPUID_BRAND_STRING2, (UINT32 *)&(mMessage[0x10]), (UINT32 *)&(mMessage[0x14]), (UINT32 *)&(mMessage[0x18]), (UINT32 *)&(mMessage[0x1C]));
  AsmCpuid (CPUID_BRAND_STRING3, (UINT32 *)&(mMessage[0x20]), (UINT32 *)&(mMessage[0x24]), (UINT32 *)&(mMessage[0x28]), (UINT32 *)&(mMessage[0x2C]));

  // Make sure mMessage is terminated and return
  mMessage[0x30] = 0x00;
  return mMessage;
}

/**
  This function returns a ChAR8* with the CPUID EAX value in hex format.
**/
CHAR8 *
GetCpuIdFamilyEaxString (
  VOID
  )
{
  UINT32  RegEax;
  UINTN   Len;

  AsmCpuid (CPUID_VERSION_INFO, &RegEax, NULL, NULL, NULL);

  Len = AsciiSPrint (mMessage, MAX_MESSAGE_LENGTH, "0x%08x", RegEax);
  return ((Len == 0) ? NULL : mMessage);
}

/**
  Driver entry point

  @param  ImageHandle   ImageHandle of the loaded driver.
  @param  SystemTable   Pointer to the EFI System Table.

  @retval EFI_SUCCESS   The handlers were registered successfully.
**/
EFI_STATUS
EFIAPI
CpuInfoDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DEBUG ((DEBUG_INFO, "[%a] - Entry\n", __FUNCTION__));
  DEBUG ((DEBUG_INFO, "\tCPU Brand Name: %a\n", GetCpuBrandString ()));
  DEBUG ((DEBUG_INFO, "\tFamily Id: 0x%x\n", GetCpuIdFamilyEaxString ()));
  DEBUG ((DEBUG_INFO, "\tStepping: 0x%x\n", GetCpuSteppingId ()));
  DEBUG ((DEBUG_INFO, "\tModel Id: 0x%x\n", GetCpuFamilyModel ()));
  return EFI_SUCCESS;
}
