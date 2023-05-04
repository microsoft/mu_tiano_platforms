/** @file
DfciDeviceIdSupportLib.c

This library provides access to platform data the becomes the DFCI DeviceId.

Copyright (C) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>

#include <Protocol/Smbios.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DfciDeviceIdSupportLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#define TEST_MAX_STRING_SIZE  65

CHAR8    gManufacturer[TEST_MAX_STRING_SIZE] = "Size Error";
CHAR8    gProductName[TEST_MAX_STRING_SIZE]  = "Size Error";
CHAR8    gSerialNumber[TEST_MAX_STRING_SIZE] = "Size Error";
BOOLEAN  gInitialized                        = FALSE;

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

  TempPtr = StringPtr;
  while ((*TempPtr != '\0') && (--Index > 0)) {
    while (*TempPtr++ != '\0') {
    }
  }

  return TempPtr;
}

/**
  GetIdString   - Return the string requested in its own allocated buffer

@param[in] IdString         - Address of string to return to caller
@param[in] DfciIdString     - Pointer to pointer that will receive the newly allocated string buffer
@param[in] DfciIdStringSize - Address where to store the size of the string

@return EFI_SUCCESS - String buffer returned to caller
@return EFI_ERROR   - Error the string

**/
STATIC
EFI_STATUS
GetIdString (
  CHAR8  *IdString,
  CHAR8  **DfciIdString,
  UINTN  *DfciIdStringSize   OPTIONAL
  )
{
  CHAR8                *BiosString;
  EFI_SMBIOS_PROTOCOL  *Smbios;
  EFI_SMBIOS_TYPE      SmbiosType;
  EFI_SMBIOS_HANDLE    SmbiosHandle;
  SMBIOS_STRUCTURE     *SmbiosRecord;
  SMBIOS_TABLE_TYPE1   *SmbiosType1;
  EFI_STATUS           Status;
  CHAR8                *StringPtr;
  UINTN                StrSize;

  Status = EFI_SUCCESS;

  if (!gInitialized) {
    gInitialized = TRUE;

    DEBUG ((DEBUG_ERROR, "%a: Entry\n", __FUNCTION__));

    Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID *)&Smbios);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: Unable to locate SmBiosProtocol.  Code=%r\n", __FUNCTION__, Status));
      goto Exit;
    }

    SmbiosType   = SMBIOS_TYPE_SYSTEM_INFORMATION;
    SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;

    // -smbios type=1, manufacturer=Palindrome, product=MuQemuQ35, serial=42-42-42-42"

    Status = Smbios->GetNext (Smbios, &SmbiosHandle, &SmbiosType, &SmbiosRecord, NULL);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: Unable to get type 1 SMBIOS record.  Code=%r\n", __FUNCTION__, Status));
      goto Exit;
    }

    SmbiosType1 = (SMBIOS_TABLE_TYPE1 *)SmbiosRecord;
    StringPtr   = (CHAR8 *)SmbiosType1 + SmbiosType1->Hdr.Length;

    DEBUG ((DEBUG_INFO, "Type 1 = %p, Size=0x%x, String = %p\n", SmbiosType1, SmbiosType1->Hdr.Length, StringPtr));
    DUMP_HEX (DEBUG_INFO, 0, StringPtr, ((UINTN)StringPtr | 0xFFF) - (UINTN)StringPtr, "");

    BiosString = GetBiosString (StringPtr, SmbiosType1->Manufacturer);
    Status     = AsciiStrCpyS (gManufacturer, sizeof (gManufacturer), BiosString);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: Manufacturer error.  Code = %r\n", __FUNCTION__, Status));
      goto Exit;
    }

    BiosString = GetBiosString (StringPtr, SmbiosType1->ProductName);
    Status     = AsciiStrCpyS (gProductName, sizeof (gProductName), BiosString);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: ProductName error.  Code = %r\n", __FUNCTION__, Status));
      goto Exit;
    }

    BiosString = GetBiosString (StringPtr, SmbiosType1->SerialNumber);
    Status     = AsciiStrCpyS (gSerialNumber, sizeof (gSerialNumber), BiosString);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: SerialNumber error.  Code = %r\n", __FUNCTION__, Status));
      goto Exit;
    }

    DEBUG ((DEBUG_INFO, "%a: DfciDeviceId:\n", __FUNCTION__));
    DEBUG ((DEBUG_INFO, "%a:     Manufacturer  = %a\n", __FUNCTION__, gManufacturer));
    DEBUG ((DEBUG_INFO, "%a:     Product Name  = %a\n", __FUNCTION__, gProductName));
    DEBUG ((DEBUG_INFO, "%a:     Serial Number = %a\n", __FUNCTION__, gSerialNumber));
  }

  if (DfciIdString == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  StrSize       = AsciiStrSize (IdString);
  *DfciIdString = AllocateCopyPool (StrSize, IdString);
  if (DfciIdStringSize != NULL) {
    *DfciIdStringSize = StrSize;
  }

  if (*DfciIdString == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Unable to allocate storage for IdString(%a).\n", __FUNCTION__, IdString));
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

Exit:
  return Status;
}

/**
Gets the serial number for this device.

@para[out] SerialNumber - UINTN value of serial number

@return EFI_SUCCESS - SerialNumber has been updated to equal the serial number of the device
@return EFI_ERROR   - Error getting number

**/
EFI_STATUS
EFIAPI
DfciIdSupportV1GetSerialNumber (
  OUT UINTN  *SerialNumber
  )
{
  return EFI_UNSUPPORTED;
}

/**
 * Get the Manufacturer Name
 *
 * @param Manufacturer
 * @param ManufacturerSize
 *
 * It is the callers responsibility to free the buffer returned.
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciIdSupportGetManufacturer (
  CHAR8  **Manufacturer,
  UINTN  *ManufacturerSize   OPTIONAL
  )
{
  EFI_STATUS  Status;

  Status = GetIdString (gManufacturer, Manufacturer, ManufacturerSize);
  return Status;
}

/**
 * Get the ProductName
 *
 * @param ProductName
 * @param ProductNameSize
 *
 * It is the callers responsibility to free the buffer returned.
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciIdSupportGetProductName (
  CHAR8  **ProductName,
  UINTN  *ProductNameSize  OPTIONAL
  )
{
  EFI_STATUS  Status;

  Status = GetIdString (gProductName, ProductName, ProductNameSize);
  return Status;
}

/**
 * Get the SerialNumber
 *
 * @param SerialNumber
 * @param SerialNumberSize
 *
 * It is the callers responsibility to free the buffer returned.
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciIdSupportGetSerialNumber (
  CHAR8  **SerialNumber,
  UINTN  *SerialNumberSize  OPTIONAL
  )
{
  EFI_STATUS  Status;

  Status = GetIdString (gSerialNumber, SerialNumber, SerialNumberSize);
  return Status;
}
