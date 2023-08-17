/** @file CpuInfoDxe.c

    Copyright (c) Microsoft Corporation. All rights reserved.

    Driver that prints CPU branding information
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiCpuLib.h>

typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32    SteppingId       : 4; ///< [Bits   3:0] Stepping ID
    UINT32    Model            : 4; ///< [Bits   7:4] Model
    UINT32    FamilyId         : 4; ///< [Bits  11:8] Family
    UINT32    ProcessorType    : 2; ///< [Bits 13:12] Processor Type
    UINT32    Reserved1        : 2; ///< [Bits 15:14] Reserved
    UINT32    ExtendedModelId  : 4; ///< [Bits 19:16] Extended Model ID
    UINT32    ExtendedFamilyId : 8; ///< [Bits 27:20] Extended Family ID
    UINT32    Reserved2        : 4; ///< Reserved
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32    Uint32;
} CPUID_VERSION_INFO_EAX;

#define CPUID_VERSION_INFO   0x01
#define CPUID_BRAND_STRING1  0x80000002
#define CPUID_BRAND_STRING2  0x80000003
#define CPUID_BRAND_STRING3  0x80000004
#define MAX_MESSAGE_LENGTH   64

STATIC CHAR8  mMessage[MAX_MESSAGE_LENGTH];

CHAR8 *
EFIAPI
GetCpuBrandString (
  )
{
  // Needed length check, 3 CPUID calls with 4 DWORDs of chars and a NULL termination
  if (MAX_MESSAGE_LENGTH < ((3 * 4 * 4) + 1)) {
    ASSERT (FALSE);
    return NULL;
  }

  // Update CPUID data (ASCII chars) directly into mMessage buffer
  AsmCpuid (CPUID_BRAND_STRING1, (UINT32 *)&(mMessage[0x00]), (UINT32 *)&(mMessage[0x04]), (UINT32 *)&(mMessage[0x08]), (UINT32 *)&(mMessage[0x0C]));
  AsmCpuid (CPUID_BRAND_STRING2, (UINT32 *)&(mMessage[0x10]), (UINT32 *)&(mMessage[0x14]), (UINT32 *)&(mMessage[0x18]), (UINT32 *)&(mMessage[0x1C]));
  AsmCpuid (CPUID_BRAND_STRING3, (UINT32 *)&(mMessage[0x20]), (UINT32 *)&(mMessage[0x24]), (UINT32 *)&(mMessage[0x28]), (UINT32 *)&(mMessage[0x2C]));

  // Make sure mMessage is terminated and return
  mMessage[0x30] = 0x00;
  return mMessage;
}

CHAR8 *
EFIAPI
GetCpuIdFamiyEaxString (
  )
{
  UINT32  RegEax;
  UINTN   Len;

  AsmCpuid (CPUID_VERSION_INFO, &RegEax, NULL, NULL, NULL);

  Len = AsciiSPrint (mMessage, MAX_MESSAGE_LENGTH, "0x%08x", RegEax);
  return ((Len == 0) ? NULL : mMessage);
}

UINT8
EFIAPI
GetCpuSteppingId (
  VOID
  )
{
  CPUID_VERSION_INFO_EAX  Eax;

  AsmCpuid (CPUID_VERSION_INFO, &Eax.Uint32, NULL, NULL, NULL);

  return (UINT8)Eax.Bits.SteppingId;
}

UINT32
EFIAPI
GetCpuFamilyModel (
  VOID
  )
{
  CPUID_VERSION_INFO_EAX  Eax;

  AsmCpuid (CPUID_VERSION_INFO, &Eax.Uint32, NULL, NULL, NULL);

  //
  // Mask other fields than Family and Model.
  //
  Eax.Bits.SteppingId    = 0;
  Eax.Bits.ProcessorType = 0;
  Eax.Bits.Reserved1     = 0;
  Eax.Bits.Reserved2     = 0;
  return Eax.Uint32;
}

/**
   Dxe Entry Point
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
  DEBUG ((DEBUG_INFO, "\tFamily Id: %x\n", GetCpuIdFamiyEaxString ()));
  DEBUG ((DEBUG_INFO, "\tStepping: %x\n", GetCpuSteppingId ()));
  DEBUG ((DEBUG_INFO, "\tModel Id: %x\n", GetCpuFamilyModel ()));
  return EFI_SUCCESS;
}
