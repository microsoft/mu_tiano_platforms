/* Test Driver to see if Garand can get it to work */

#include <Library/DebugLib.h> // Print
#include <Library/BaseLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

/**
  Entry point function of this shell application.
**/
EFI_STATUS
EFIAPI
TestAppMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
    Print (L"Test App can run\n");
    return 0;
}