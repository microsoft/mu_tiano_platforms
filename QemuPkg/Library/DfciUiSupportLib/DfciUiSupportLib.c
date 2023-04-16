/** @file
DfciUiSupportLib.c

Qemu instance of the UiSupportLib.

Copyright (C) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Protocol/SimpleWindowManager.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DfciUiSupportLib.h>
#include <Library/SwmDialogsLib.h>
#include <Library/UefiLib.h>

/**
  This routine indicates if the system is in Manufacturing Mode.

  @retval  ManufacturingMode - Platforms may have a manufacturing mode.
                               DFCI Auto opt-in's the management cert included
                               in the firmware volume in Manufacturing Mode.
                               TRUE if the device is in Manufacturing Mode
**/
BOOLEAN
EFIAPI
DfciUiIsManufacturingMode (
  VOID
  )
{
  return FALSE;
}

/**

  This routine indicates if the UI is ready and can be used.

  @param   ManufacturingMode - Platforms may have a manufacturing mode.
                               DFCI Auto opt-in's the management cert included
                               in the firmware volume in Manufacturing Mode.
                               TRUE if the device is in Manufacturing Mode

  @retval  TRUE if the UI is ready to use, else FALSE.

**/
BOOLEAN
EFIAPI
DfciUiIsUiAvailable (
  VOID
  )
{
  return SwmDialogsReady ();
}

/**
 * Display a Message Box
 *
 * NOTE: The UI must be available
 *
 * @param[in] TitleBarText
 * @param[in] Text
 * @param[in] Caption
 * @param[in] Type
 * @param[in] Timeout
 * @param[out] Result
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciUiDisplayMessageBox (
  IN  CHAR16          *TitleBarText,
  IN  CHAR16          *Text,
  IN  CHAR16          *Caption,
  IN  UINT32          Type,
  IN  UINT64          Timeout,
  OUT DFCI_MB_RESULT  *Result
  )
{
  EFI_STATUS     Status;
  SWM_MB_RESULT  SwmResult;

  Status = SwmDialogsMessageBox (
             TitleBarText,
             Caption,
             Text,
             Type,
             Timeout,
             &SwmResult
             );

  if (!EFI_ERROR (Status)) {
    *Result = (DFCI_MB_RESULT)SwmResult;
  } else {
    DEBUG ((DEBUG_ERROR, "%a: SwmDialogBox error code=%r\n", __FUNCTION__, Status));
  }

  return Status;
}

/**
 * Display a Message Box
 *
 * NOTE: The UI must be available
 *
 * @param[in] TitleText
 * @param[in] CapionText
 * @param[in] BodyText
 * @param[in] ErrorText
 * @param[out] Result
 * @param[out] Password
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciUiDisplayPasswordDialog (
  IN  CHAR16          *TitleText,
  IN  CHAR16          *CaptionText,
  IN  CHAR16          *BodyText,
  IN  CHAR16          *ErrorText,
  OUT DFCI_MB_RESULT  *Result,
  OUT CHAR16          **Password
  )
{
  EFI_STATUS     Status;
  SWM_MB_RESULT  SwmResult;

  Status = SwmDialogsPasswordPrompt (
             TitleText,
             CaptionText,
             BodyText,
             ErrorText,
             SWM_MB_STYLE_NORMAL,
             &SwmResult,
             Password
             );

  if (!EFI_ERROR (Status)) {
    *Result = (DFCI_MB_RESULT)SwmResult;
  } else {
    DEBUG ((DEBUG_ERROR, "%a: SwmDialogsPasswordPrompt error code=%r\n", __FUNCTION__, Status));
  }

  return Status;
}

/**
 * DfciUiDisplayDfciAuthDialog
 *
 * @param[in] TitleText
 * @param[in] CaptionText
 * @param[in] BodyText
 * @param[in] CertText
 * @param[in] ConfirmText
 * @param[in] ErrorText
 * @param[in] PasswordType
 * @param[in] Result
 * @param[out] OPTIONAL
 * @param[out] OPTIONAL
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciUiDisplayAuthDialog (
  IN  CHAR16          *TitleText,
  IN  CHAR16          *CaptionText,
  IN  CHAR16          *BodyText,
  IN  CHAR16          *CertText,
  IN  CHAR16          *ConfirmText,
  IN  CHAR16          *ErrorText,
  IN  BOOLEAN         PasswordType,
  IN  CHAR16          *Thumbprint,
  OUT DFCI_MB_RESULT  *Result,
  OUT CHAR16          **Password OPTIONAL
  )
{
  EFI_STATUS     Status;
  SWM_MB_RESULT  SwmResult;
  CHAR16         *ThumbprintResponse;

  Status = SwmDialogsVerifyThumbprintPrompt (
             TitleText,
             CaptionText,
             BodyText,
             CertText,
             ConfirmText,
             ErrorText,
             PasswordType ? SWM_THMB_TYPE_ALERT_PASSWORD : SWM_THMB_TYPE_ALERT_THUMBPRINT,
             &SwmResult,
             Password,
             &ThumbprintResponse
             );

  if (!EFI_ERROR (Status)) {
    if (0 != StrnCmp (Thumbprint, ThumbprintResponse, sizeof (UINT16) * 3)) {
      // Max is two characters and a NULL
      Status = EFI_SECURITY_VIOLATION;
    } else {
      *Result = (DFCI_MB_RESULT)SwmResult;
    }
  } else {
    DEBUG ((DEBUG_ERROR, "%a: SwmDialogsVerifyThumbprintPrompt error code=%r\n", __FUNCTION__, Status));
  }

  return Status;
}

/**
    DfciUiExitSecurityBoundary

    UEFI that support locked settings variables can lock those
    variable when this function is called.  DFCI will call this function
    before enabling USB or the Network device which are considered unsafe.

    Signal PreReadyToBoot - lock private settings variable to insure
           USB or Network don't have access to locked settings.
    Disable the OSK from displaying (PreReadyToBoot also enables the OSK)
**/
VOID
EFIAPI
DfciUiExitSecurityBoundary (
  VOID
  )
{
  EfiEventGroupSignal (&gEfiEventPreReadyToBootGuid);

  return;
}
