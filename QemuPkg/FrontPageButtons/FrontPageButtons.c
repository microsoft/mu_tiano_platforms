/** @file FrontPageButtons.c

 This module installs the MsButtonServicesProtocol and reports the requested state of Vol+ and
 Vol- as indicated by the smbios table type3 record version string.

 If the Type3.version string is:
     "Vol+"  -- Return Vol+ pressed.
     "Vol-"  -- Return Vol- pressed.
     any other string, or no string, return neither pressed

  Use only for Qemu platforms which can change smbios every time it boots.

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>

#include <Protocol/ButtonServices.h>
#include <Protocol/Smbios.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

typedef enum {
  NoButtons     = 0,
  VolUpButton   = 1,
  VolDownButton = 2
} BUTTON_STATE;

#define SMBIOS_VOLUME_UP    "Vol+"
#define SMBIOS_VOLUME_DOWN  "Vol-"

BUTTON_STATE  gButtonState = NoButtons;

/*
Say volume up button is pressed because we want to go to Front Page.

@param[in]     - Button Services protocol pointer
@param[out]    - Pointer to a boolean value to receive the button state

@retval        - EFI_SUCCESS;
*/
EFI_STATUS
EFIAPI
PreBootVolumeUpButtonThenPowerButtonCheck (
  IN  MS_BUTTON_SERVICES_PROTOCOL  *This,
  OUT BOOLEAN                      *PreBootVolumeUpButtonThenPowerButton // TRUE if button combo set else FALSE
  )
{
  DEBUG ((DEBUG_ERROR, "%a \n", __FUNCTION__));
  *PreBootVolumeUpButtonThenPowerButton = (gButtonState == VolUpButton);
  return EFI_SUCCESS;
}

/*
Say volume down button is pressed because we want alt boot.

@param[in]     - Button Services protocol pointer
@param[out]    - Pointer to a boolean value to receive the button state

@retval        - EFI_SUCCESS;
*/
EFI_STATUS
EFIAPI
PreBootVolumeDownButtonThenPowerButtonCheck (
  IN  MS_BUTTON_SERVICES_PROTOCOL  *This,
  OUT BOOLEAN                      *PreBootVolumeDownButtonThenPowerButton // TRUE if button combo set else FALSE
  )
{
  DEBUG ((DEBUG_ERROR, "%a \n", __FUNCTION__));
  *PreBootVolumeDownButtonThenPowerButton = (gButtonState == VolDownButton);
  return EFI_SUCCESS;
}

/*
Clear current button state.

@param[in]     - Button Services protocol pointer

@retval        - EFI_SUCCESS;
*/
EFI_STATUS
EFIAPI
PreBootClearVolumeButtonState (
  MS_BUTTON_SERVICES_PROTOCOL  *This
  )
{
  DEBUG ((DEBUG_ERROR, "%a \n", __FUNCTION__));
  gButtonState = NoButtons;

  return EFI_SUCCESS;
}

/**
  Get Bios String  - Return address of Bios String

**/
STATIC
CHAR8 *
GetBiosString (
  CHAR8  *StringPtr,
  INTN   Index
  )
{
  CHAR8  *TempPtr;

  if (Index == 0) {
    return NULL;
  }

  TempPtr = StringPtr;
  while ((*TempPtr != '\0') && (--Index > 0)) {
    while (*TempPtr++ != '\0') {
    }
  }

  return TempPtr;
}

/**
GetButtonState gets the button state of the Vol+/Vol- buttons from the SMBIOS Table, and stores that
state in gButtonState.

@return EFI_SUCCESS - String buffer returned to caller
@return EFI_ERROR   - Error the string

**/
EFI_STATUS
GetButtonState (
  VOID
  )
{
  CHAR8                *BiosString;
  EFI_SMBIOS_PROTOCOL  *Smbios;
  EFI_SMBIOS_TYPE      SmbiosType;
  EFI_SMBIOS_HANDLE    SmbiosHandle;
  SMBIOS_STRUCTURE     *SmbiosRecord;
  SMBIOS_TABLE_TYPE3   *SmbiosType3;
  EFI_STATUS           Status;
  CHAR8                *StringPtr;
  UINTN                StrLen;

  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_ERROR, "%a: Entry\n", __FUNCTION__));

  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID *)&Smbios);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Unable to locate SmBiosProtocol.  Code=%r\n", __FUNCTION__, Status));
    goto Exit;
  }

  SmbiosType   = SMBIOS_TYPE_SYSTEM_ENCLOSURE;
  SmbiosHandle = 0xFFFe;

  // -smbios type=3,version=V+

  Status = Smbios->GetNext (Smbios, &SmbiosHandle, &SmbiosType, &SmbiosRecord, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Unable to get type 3 SMBIOS record.  Code=%r\n", __FUNCTION__, Status));
    goto Exit;
  }

  SmbiosType3 = (SMBIOS_TABLE_TYPE3 *)SmbiosRecord;
  StringPtr   = (CHAR8 *)SmbiosType3 + SmbiosType3->Hdr.Length;

  DEBUG ((DEBUG_INFO, "Type 3 = %p, Size=0x%x, String = %p\n", SmbiosType3, SmbiosType3->Hdr.Length, StringPtr));
  DUMP_HEX (DEBUG_INFO, 0, SmbiosType3, ((UINTN)StringPtr | 0xFFF) - (UINTN)SmbiosType3, "");

  BiosString = GetBiosString (StringPtr, SmbiosType3->Version);

  if (BiosString != NULL) {
    StrLen = AsciiStrLen (BiosString);

    if ((StrLen == AsciiStrLen (SMBIOS_VOLUME_UP))  &&
        (0 == AsciiStrCmp (BiosString, SMBIOS_VOLUME_UP)))
    {
      gButtonState = VolUpButton;
      DEBUG ((DEBUG_INFO, "%a: Vol+ Button Detected\n", __FUNCTION__));
    }

    if ((StrLen == AsciiStrLen (SMBIOS_VOLUME_DOWN)) &&
        (0 == AsciiStrCmp (BiosString, SMBIOS_VOLUME_DOWN)))
    {
      gButtonState = VolDownButton;
      DEBUG ((DEBUG_INFO, "%a: Vol- Button Detected\n", __FUNCTION__));
    }
  }

  if (gButtonState == NoButtons) {
    DEBUG ((DEBUG_INFO, "%a: Neither Vol+ nor Vol- detected\n", __FUNCTION__));
  }

Exit:
  return Status;
}

/**
 Constructor routine to install button services protocol and initialize anything related to buttons


@param[in]     - Image Handle of the process loading this module
@param[in]     - Efi System Table pointer

@retval        - EFI_SUCCESS (always for a constructor)
 **/
EFI_STATUS
EFIAPI
ButtonsInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  MS_BUTTON_SERVICES_PROTOCOL  *Protocol = NULL;
  EFI_STATUS                   Status    = EFI_SUCCESS;

  DEBUG ((DEBUG_ERROR, "%a \n", __FUNCTION__));

  Status = GetButtonState ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Protocol = AllocateZeroPool (sizeof (MS_BUTTON_SERVICES_PROTOCOL));
  if (Protocol == NULL) {
    DEBUG ((DEBUG_ERROR, "Failed to allocate memory for button service protocol.\n"));
    return EFI_SUCCESS;
  }

  Protocol->PreBootVolumeDownButtonThenPowerButtonCheck = PreBootVolumeDownButtonThenPowerButtonCheck;
  Protocol->PreBootVolumeUpButtonThenPowerButtonCheck   = PreBootVolumeUpButtonThenPowerButtonCheck;
  Protocol->PreBootClearVolumeButtonState               = PreBootClearVolumeButtonState;

  // Install the protocol
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gMsButtonServicesProtocolGuid,
                  Protocol,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Button Services Protocol Publisher: install protocol error, Status = %r.\n", Status));
    FreePool (Protocol);
    return EFI_SUCCESS;
  }

  DEBUG ((DEBUG_INFO, "Button Services Protocol Installed!\n"));
  return EFI_SUCCESS;
}
