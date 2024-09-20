/** @file
  OVMF ACPI Platform Driver

  Copyright (c) 2008 - 2012, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef ACPI_PLATFORM_H_
#define ACPI_PLATFORM_H_

#include <Protocol/AcpiTable.h> // EFI_ACPI_TABLE_PROTOCOL
#include <Protocol/PciIo.h>     // EFI_PCI_IO_PROTOCOL

typedef struct {
  EFI_PCI_IO_PROTOCOL    *PciIo;
  UINT64                 PciAttributes;
} ORIGINAL_ATTRIBUTES;

EFI_STATUS
EFIAPI
InstallCloudHvTables (
  IN   EFI_ACPI_TABLE_PROTOCOL  *AcpiProtocol
  );

EFI_STATUS
EFIAPI
InstallQemuFwCfgTables (
  IN   EFI_ACPI_TABLE_PROTOCOL  *AcpiProtocol
  );

EFI_STATUS
EFIAPI
InstallAcpiTables (
  IN   EFI_ACPI_TABLE_PROTOCOL  *AcpiTable
  );

VOID
EnablePciDecoding (
  OUT ORIGINAL_ATTRIBUTES  **OriginalAttributes,
  OUT UINTN                *Count
  );

VOID
RestorePciDecoding (
  IN ORIGINAL_ATTRIBUTES  *OriginalAttributes,
  IN UINTN                Count
  );

#endif
