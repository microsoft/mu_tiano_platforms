/** @file
 *DeviceBootManager  - Ms Device specific extensions to BdsDxe.

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <Guid/EventGroup.h>
#include <Guid/GlobalVariable.h>

#include <Protocol/TpmPpProtocol.h>

#include <Library/BaseMemoryLib.h>
#include <Library/BootGraphicsLib.h>
#include <Library/ConsoleMsgLib.h>
#include <Library/DebugLib.h>
#include <Library/DeviceBootManagerLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MsBootOptionsLib.h>
#include <Library/MsBootPolicyLib.h>
#include <Library/MsNetworkDependencyLib.h>
#include <Library/MsPlatformDevicesLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/Tcg2PhysicalPresenceLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Settings/BootMenuSettings.h>

static EFI_EVENT  mPostReadyToBootEvent;

CHAR8  mMemoryType[][30] = {
  // Value for PcdMemoryMapTypes
  "EfiReservedMemoryType      ",                               // 0x0001
  "EfiLoaderCode              ",                               // 0x0002
  "EfiLoaderData              ",                               // 0x0004
  "EfiBootServicesCode        ",                               // 0x0008
  "EfiBootServicesData        ",                               // 0x0010
  "EfiRuntimeServicesCode     ",                               // 0x0020
  "EfiRuntimeServicesData     ",                               // 0x0040
  "EfiConventionalMemory      ",                               // 0x0080
  "EfiUnusableMemory          ",                               // 0x0100
  "EfiACPIReclaimMemory       ",                               // 0x0200   Both ACPI types would
  "EfiACPIMemoryNVS           ",                               // 0x0400   be 0x0600
  "EfiMemoryMappedIO          ",                               // 0x0800
  "EfiMemoryMappedIOPortSpace ",                               // 0x1000
  "EfiPalCode                 ",                               // 0x2000
  "EfiMaxMemoryType           "
};

/**
 * Print Memory Map
 *
 * @return VOID
 */
static
VOID
PrintMemoryMap (
  )
{
  if (PcdGet8 (PcdEnableMemMapOutput)) {
    EFI_STATUS             Status;
    UINTN                  MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR  *MemoryMap    = NULL;
    VOID                   *p;
    CHAR8                  *Entry;
    UINTN                  MapKey;
    UINTN                  DescriptorSize;
    UINT32                 DescriptorVersion;
    UINTN                  Count;
    UINTN                  i;

    Status = gBS->GetMemoryMap (&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      MemoryMap = AllocatePool (MemoryMapSize + (sizeof (EFI_MEMORY_DESCRIPTOR) * 2));
      p         = (VOID *)MemoryMap; // Save for FreePool
      if (MemoryMap != NULL) {
        Status = gBS->GetMemoryMap (&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
        Entry  = (CHAR8 *)MemoryMap;
        if (Status == EFI_SUCCESS) {
          Count = MemoryMapSize / DescriptorSize;
          for (i = 0; i < Count; i++) {
            MemoryMap = (EFI_MEMORY_DESCRIPTOR *)Entry;
            if (MemoryMap->Type <= EfiMaxMemoryType) {
              if (((1 << MemoryMap->Type) & PcdGet32 (PcdEnableMemMapTypes)) != 0) {
                DEBUG ((DEBUG_INFO, "%a at %p for %d pages\n", mMemoryType[MemoryMap->Type], MemoryMap->PhysicalStart, MemoryMap->NumberOfPages));
                if (PcdGet8 (PcdEnableMemMapDumpOutput)) {
                  DUMP_HEX (DEBUG_INFO, 0, (CHAR8 *)MemoryMap->PhysicalStart, 48, "");
                }
              }
            } else {
              DEBUG ((DEBUG_ERROR, "Invalid memory type - %x\n", MemoryMap->Type));
            }

            Entry += DescriptorSize;
          }
        }

        FreePool (p);
      }
    }
  }
}

/**
  Check if current BootCurrent variable is internal shell boot option.

  @retval  TRUE         BootCurrent is internal shell.
  @retval  FALSE        BootCurrent is not internal shell.
**/
static
BOOLEAN
BootCurrentIsInternalShell (
  VOID
  )
{
  UINTN                     VarSize;
  UINT16                    BootCurrent;
  CHAR16                    BootOptionName[16];
  UINT8                     *BootOption;
  UINT8                     *Ptr;
  BOOLEAN                   Result;
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *LastDeviceNode;
  EFI_GUID                  *GuidPoint;

  BootOption = NULL;
  Result     = FALSE;

  //
  // Get BootCurrent variable
  //
  VarSize = sizeof (UINT16);
  Status  = gRT->GetVariable (
                   L"BootCurrent",
                   &gEfiGlobalVariableGuid,
                   NULL,
                   &VarSize,
                   &BootCurrent
                   );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // Create boot option Bootxxxx from BootCurrent
  //
  UnicodeSPrint (BootOptionName, sizeof (BootOptionName), L"Boot%04x", BootCurrent);

  GetEfiGlobalVariable2 (BootOptionName, (VOID **)&BootOption, &VarSize);

  if ((BootOption == NULL) || (VarSize == 0)) {
    return FALSE;
  }

  Ptr            = BootOption;
  Ptr           += sizeof (UINT32);
  Ptr           += sizeof (UINT16);
  Ptr           += StrSize ((CHAR16 *)Ptr);
  TempDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)Ptr;
  LastDeviceNode = TempDevicePath;
  while (!IsDevicePathEnd (TempDevicePath)) {
    LastDeviceNode = TempDevicePath;
    TempDevicePath = NextDevicePathNode (TempDevicePath);
  }

  GuidPoint = EfiGetNameGuidFromFwVolDevicePathNode (
                (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *)LastDeviceNode
                );
  if ((GuidPoint != NULL) &&
      ((CompareGuid (GuidPoint, PcdGetPtr (PcdShellFile))) ||
       (CompareGuid (GuidPoint, &gUefiShellFileGuid)))
      )
  {
    //
    // if this option is internal shell, return TRUE
    //
    Result = TRUE;
  }

  if (BootOption != NULL) {
    FreePool (BootOption);
    BootOption = NULL;
  }

  return Result;
}

/**
Post ready to boot callback to print memory map, and update FACS hardware signature.
For booting the internal shell, set the video resolution to low.

@param  Event                 Event whose notification function is being invoked.
@param  Context               The pointer to the notification function's context,
which is implementation-dependent.
**/
static
VOID
EFIAPI
PostReadyToBoot (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  if (BootCurrentIsInternalShell ()) {
    EfiBootManagerConnectAll ();
  }

  StartNetworking ();

  PrintMemoryMap ();

  return;
}

/**
 * Constructor   - This runs when BdsDxe is loaded, before BdsArch protocol is published
 *
 * @return EFI_STATUS
 */
EFI_STATUS
EFIAPI
DeviceBootManagerConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  PostReadyToBoot,
                  NULL,
                  &gEfiEventAfterReadyToBootGuid,
                  &mPostReadyToBootEvent
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR [BDS]: Failed to register OnReadyToBoot (%r).\r\n", Status));
  }

  // Constructor MUST return EFI_SUCCESS as a failure can result in an unusable system
  return EFI_SUCCESS;
}

/**
  OnDemandConInConnect
 */
EFI_DEVICE_PATH_PROTOCOL **
EFIAPI
DeviceBootManagerOnDemandConInConnect (
  VOID
  )
{
  return GetPlatformConnectOnConInList ();
}

/**
  Do the device specific action at start of BdsEntry (callback into BdsArch from DXE Dispatcher)
**/
VOID
EFIAPI
DeviceBootManagerBdsEntry (
  VOID
  )
{
  EfiEventGroupSignal (&gMsStartOfBdsNotifyGuid);
}

/**
  Do the device specific action before the console is connected.

  Such as:

      Initialize the platform boot order
      Supply Console information

  Returns value from GetPlatformPreffered Console, which will be the handle and device path
  of the device console
**/
EFI_HANDLE
EFIAPI
DeviceBootManagerBeforeConsole (
  EFI_DEVICE_PATH_PROTOCOL   **DevicePath,
  BDS_CONSOLE_CONNECT_ENTRY  **PlatformConsoles
  )
{
  MsBootOptionsLibRegisterDefaultBootOptions ();
  *PlatformConsoles = GetPlatformConsoleList ();

  return GetPlatformPreferredConsole (DevicePath);
}

/**
  Do the device specific action after the console is connected.

  Such as:
**/
EFI_DEVICE_PATH_PROTOCOL **
EFIAPI
DeviceBootManagerAfterConsole (
  VOID
  )
{
  EFI_BOOT_MODE    BootMode;
  TPM_PP_PROTOCOL  *TpmPp = NULL;
  EFI_STATUS       Status;

  Status = DisplayBootGraphic (BG_SYSTEM_LOGO);
  if (EFI_ERROR (Status) != FALSE) {
    DEBUG ((DEBUG_ERROR, "%a Unabled to set graphics - %r\n", __func__, Status));
  }

  ConsoleMsgLibDisplaySystemInfoOnConsole ();

  BootMode = GetBootModeHob ();

  if (BootMode != BOOT_ON_FLASH_UPDATE) {
    Status = gBS->LocateProtocol (&gTpmPpProtocolGuid, NULL, (VOID **)&TpmPp);
    if (!EFI_ERROR (Status) && (TpmPp != NULL)) {
      Status = TpmPp->PromptForConfirmation (TpmPp);
      DEBUG ((DEBUG_ERROR, "%a: Unexpected return from Tpm Physical Presence. Code=%r\n", __func__, Status));
    }

    Tcg2PhysicalPresenceLibProcessRequest (NULL);
  }

  return GetPlatformConnectList ();
}

/**
ProcessBootCompletion
*/
VOID
EFIAPI
DeviceBootManagerProcessBootCompletion (
  IN EFI_BOOT_MANAGER_LOAD_OPTION  *BootOption
  )
{
  return;
}

/**
 * Check for HardKeys during boot.  If the hard keys are pressed, builds
 * a boot option for the specific hard key setting.
 *
 *
 * @param BootOption   - Boot Option filled in based on which hard key is pressed
 *
 * @return EFI_STATUS  - EFI_NOT_FOUND - no hard key pressed, no BootOption
 *                       EFI_SUCCESS   - BootOption is valid
 *                       other error   - Unable to build BootOption
 */
EFI_STATUS
EFIAPI
DeviceBootManagerPriorityBoot (
  EFI_BOOT_MANAGER_LOAD_OPTION  *BootOption
  )
{
  BOOLEAN     AltDeviceBoot;
  EFI_STATUS  Status;

  AltDeviceBoot = MsBootPolicyLibIsAltBoot ();
  MsBootPolicyLibClearBootRequests ();

  // There are four cases:
  //   1. Nothing pressed.             return EFI_NOT_FOUND
  //   2. AltDeviceBoot                load alternate boot order
  //   3. Both indicators are present  Load NetworkUnlock

  if (AltDeviceBoot) {
    // Alternate boot or Network Unlock option
    DEBUG ((DEBUG_INFO, "[Bds] alternate boot\n"));
    Status = MsBootOptionsLibGetDefaultBootApp (BootOption, "MA");
  } else {
    Status = EFI_NOT_FOUND;
  }

  return Status;
}

/**
 This is called from BDS right before going into front page
 when no bootable devices/options found
*/
VOID
EFIAPI
DeviceBootManagerUnableToBoot (
  VOID
  )
{
  return;
}
