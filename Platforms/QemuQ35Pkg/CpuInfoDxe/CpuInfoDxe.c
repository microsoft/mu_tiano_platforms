/**
The driver's entry point.

@param[in] ImageHandle  The firmware allocated handle for the EFI image.
@param[in] SystemTable  A pointer to the EFI System Table.

@retval EFI_SUCCESS   The entry point is executed successfully.
@retval other         Some error occurs when executing this entry point.

**/
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Register/Cpuid.h>
#include <Register/Intel/ArchitecturalMsr.h>

// Globals to construct the message strings.
#define MAX_MESSAGE_LENGTH  64
STATIC CHAR8  mMessage[MAX_MESSAGE_LENGTH];

/**
  Updates mMessage with the CPU brand string as read from the CPUID instruction

  @return  mMessage pointer or NULL on error
**/
CHAR8 *
EFIAPI
GetCpuBrandString (
  VOID
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

EFI_STATUS
EFIAPI
CpuInfoDxeEntry (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DEBUG ((DEBUG_INFO, "[%a] - Entry\n", __FUNCTION__));

  DEBUG ((DEBUG_INFO, "Cpu Branding: %a \n", GetCpuBrandString()));
  DEBUG ((DEBUG_INFO, "CpuIdFamilyEaxString: %a \n", GetCpuIdFamilyEaxString()));

  return EFI_SUCCESS;
}