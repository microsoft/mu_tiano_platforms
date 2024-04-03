/** @file
  This driver is a sample implementation of the Graphics Output Protocol for
  the QEMU (Cirrus Logic 5446) video controller.

  Copyright (c) 2006 - 2019, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Qemu.h"
#include <IndustryStandard/Acpi.h>
#include <PolicyDataStructGFX.h>
#include <Protocol/Policy.h>

EFI_DRIVER_BINDING_PROTOCOL  gQemuVideoDriverBinding = {
  QemuVideoControllerDriverSupported,
  QemuVideoControllerDriverStart,
  QemuVideoControllerDriverStop,
  0x10,
  NULL,
  NULL
};

EFI_GUID  *mMsGopOverrideProtocolGuid;   // MU_CHANGE use MsGopOverrideProtocolGuid

GFX_POLICY_DATA  mGfxPolicy[GFX_PORT_MAX_CNT];

QEMU_VIDEO_CARD  gQemuVideoCardList[] = {
  {
    PCI_CLASS_DISPLAY_VGA,
    CIRRUS_LOGIC_VENDOR_ID,
    CIRRUS_LOGIC_5430_DEVICE_ID,
    QEMU_VIDEO_CIRRUS_5430,
    L"Cirrus 5430"
  },{
    PCI_CLASS_DISPLAY_VGA,
    CIRRUS_LOGIC_VENDOR_ID,
    CIRRUS_LOGIC_5430_ALTERNATE_DEVICE_ID,
    QEMU_VIDEO_CIRRUS_5430,
    L"Cirrus 5430"
  },{
    PCI_CLASS_DISPLAY_VGA,
    CIRRUS_LOGIC_VENDOR_ID,
    CIRRUS_LOGIC_5446_DEVICE_ID,
    QEMU_VIDEO_CIRRUS_5446,
    L"Cirrus 5446"
  },{
    0     /* end of list */
  }
};

static QEMU_VIDEO_CARD *
QemuVideoDetect (
  IN UINT8   SubClass,
  IN UINT16  VendorId,
  IN UINT16  DeviceId
  )
{
  UINTN  Index = 0;

  while (gQemuVideoCardList[Index].VendorId != 0) {
    if ((gQemuVideoCardList[Index].SubClass == SubClass) &&
        (gQemuVideoCardList[Index].VendorId == VendorId) &&
        (gQemuVideoCardList[Index].DeviceId == DeviceId))
    {
      return gQemuVideoCardList + Index;
    }

    Index++;
  }

  return NULL;
}

/**
  Check if this device is supported.

  @param  This                   The driver binding protocol.
  @param  Controller             The controller handle to check.
  @param  RemainingDevicePath    The remaining device path.

  @retval EFI_SUCCESS            The bus supports this controller.
  @retval EFI_UNSUPPORTED        This device isn't supported.

**/
EFI_STATUS
EFIAPI
QemuVideoControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS           Status;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  PCI_TYPE00           Pci;
  QEMU_VIDEO_CARD      *Card;

  //
  // Open the PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = EFI_UNSUPPORTED;
  if (!IS_PCI_DISPLAY (&Pci)) {
    goto Done;
  }

  Card = QemuVideoDetect (Pci.Hdr.ClassCode[1], Pci.Hdr.VendorId, Pci.Hdr.DeviceId);
  if (Card != NULL) {
    DEBUG ((DEBUG_INFO, "QemuVideo: %s detected\n", Card->Name));
    Status = EFI_SUCCESS;
  }

Done:
  //
  // Close the PCI I/O Protocol
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;
}

/**
  Start to process the controller.

  @param  This                   The USB bus driver binding instance.
  @param  Controller             The controller to check.
  @param  RemainingDevicePath    The remaining device patch.

  @retval EFI_SUCCESS            The controller is controlled by the usb bus.
  @retval EFI_ALREADY_STARTED    The controller is already controlled by the usb
                                 bus.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resources.

**/
EFI_STATUS
EFIAPI
QemuVideoControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_TPL                   OldTpl;
  EFI_STATUS                Status;
  QEMU_VIDEO_PRIVATE_DATA   *Private;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  ACPI_ADR_DEVICE_PATH      AcpiDeviceNode;
  PCI_TYPE00                Pci;
  QEMU_VIDEO_CARD           *Card;
  EFI_PCI_IO_PROTOCOL       *ChildPciIo;
  UINT64                    SupportedVgaIo;

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  //
  // Allocate Private context data for GOP interface.
  //
  Private = AllocateZeroPool (sizeof (QEMU_VIDEO_PRIVATE_DATA));
  if (Private == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto RestoreTpl;
  }

  //
  // Set up context record
  //
  Private->Signature = QEMU_VIDEO_PRIVATE_DATA_SIGNATURE;

  //
  // Open PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&Private->PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto FreePrivate;
  }

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = Private->PciIo->Pci.Read (
                                 Private->PciIo,
                                 EfiPciIoWidthUint32,
                                 0,
                                 sizeof (Pci) / sizeof (UINT32),
                                 &Pci
                                 );
  if (EFI_ERROR (Status)) {
    goto ClosePciIo;
  }

  //
  // Determine card variant.
  //
  Card = QemuVideoDetect (Pci.Hdr.ClassCode[1], Pci.Hdr.VendorId, Pci.Hdr.DeviceId);
  if (Card == NULL) {
    Status = EFI_DEVICE_ERROR;
    goto ClosePciIo;
  }

  Private->Variant = Card->Variant;

  //
  // Save original PCI attributes
  //
  Status = Private->PciIo->Attributes (
                             Private->PciIo,
                             EfiPciIoAttributeOperationGet,
                             0,
                             &Private->OriginalPciAttributes
                             );

  if (EFI_ERROR (Status)) {
    goto ClosePciIo;
  }

  //
  // Get supported PCI attributes
  //
  Status = Private->PciIo->Attributes (
                             Private->PciIo,
                             EfiPciIoAttributeOperationSupported,
                             0,
                             &SupportedVgaIo
                             );
  if (EFI_ERROR (Status)) {
    goto ClosePciIo;
  }

  SupportedVgaIo &= (UINT64)(EFI_PCI_IO_ATTRIBUTE_VGA_IO | EFI_PCI_IO_ATTRIBUTE_VGA_IO_16);
  if ((SupportedVgaIo == 0) && IS_PCI_VGA (&Pci)) {
    Status = EFI_UNSUPPORTED;
    goto ClosePciIo;
  }

  // This VGA corresponds to the 0th GFX port, so check it out.
  if (mGfxPolicy[0].Power_State_Port) {
    //
    // Set new PCI attributes
    //
    Status = Private->PciIo->Attributes (
                               Private->PciIo,
                               EfiPciIoAttributeOperationEnable,
                               EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | SupportedVgaIo,
                               NULL
                               );
    if (EFI_ERROR (Status)) {
      goto ClosePciIo;
    }
  } else {
    //
    // Set new PCI attributes
    //
    Status = Private->PciIo->Attributes (
                               Private->PciIo,
                               EfiPciIoAttributeOperationDisable,
                               EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | SupportedVgaIo,
                               NULL
                               );
    DEBUG ((DEBUG_INFO, "Done disabling target GFX device %r\n", Status));
    Status = EFI_UNSUPPORTED;
    goto ClosePciIo;
  }

  //
  // Get ParentDevicePath
  //
  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **)&ParentDevicePath
                  );
  if (EFI_ERROR (Status)) {
    goto RestoreAttributes;
  }

  //
  // Set Gop Device Path
  //
  ZeroMem (&AcpiDeviceNode, sizeof (ACPI_ADR_DEVICE_PATH));
  AcpiDeviceNode.Header.Type    = ACPI_DEVICE_PATH;
  AcpiDeviceNode.Header.SubType = ACPI_ADR_DP;
  AcpiDeviceNode.ADR            = ACPI_DISPLAY_ADR (1, 0, 0, 1, 0, ACPI_ADR_DISPLAY_TYPE_VGA, 0, 0);
  SetDevicePathNodeLength (&AcpiDeviceNode.Header, sizeof (ACPI_ADR_DEVICE_PATH));

  Private->GopDevicePath = AppendDevicePathNode (
                             ParentDevicePath,
                             (EFI_DEVICE_PATH_PROTOCOL *)&AcpiDeviceNode
                             );
  if (Private->GopDevicePath == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto RestoreAttributes;
  }

  //
  // Create new child handle and install the device path protocol on it.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,
                  &gEfiDevicePathProtocolGuid,
                  Private->GopDevicePath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto FreeGopDevicePath;
  }

  //
  // Construct video mode buffer
  //
  switch (Private->Variant) {
    case QEMU_VIDEO_CIRRUS_5430:
    case QEMU_VIDEO_CIRRUS_5446:
      Status = QemuVideoCirrusModeSetup (Private);
      break;
    default:
      ASSERT (FALSE);
      Status = EFI_DEVICE_ERROR;
      break;
  }

  if (EFI_ERROR (Status)) {
    goto UninstallGopDevicePath;
  }

  //
  // Start the GOP software stack.
  //
  Status = QemuVideoGraphicsOutputConstructor (Private);
  if (EFI_ERROR (Status)) {
    goto FreeModeData;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,
                  mMsGopOverrideProtocolGuid, // MU_CHANGE use MsGopOverrideProtocolGuid
                  &Private->GraphicsOutput,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto DestructQemuVideoGraphics;
  }

  //
  // Reference parent handle from child handle.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&ChildPciIo,
                  This->DriverBindingHandle,
                  Private->Handle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    goto UninstallGop;
  }

  gBS->RestoreTPL (OldTpl);
  return EFI_SUCCESS;

UninstallGop:
  gBS->UninstallProtocolInterface (
         Private->Handle,
         mMsGopOverrideProtocolGuid,
         &Private->GraphicsOutput
         );                                                     // MU_CHANGE use MsGopOverrideProtocolGuid

DestructQemuVideoGraphics:
  QemuVideoGraphicsOutputDestructor (Private);

FreeModeData:
  FreePool (Private->ModeData);

UninstallGopDevicePath:
  gBS->UninstallProtocolInterface (
         Private->Handle,
         &gEfiDevicePathProtocolGuid,
         Private->GopDevicePath
         );

FreeGopDevicePath:
  FreePool (Private->GopDevicePath);

RestoreAttributes:
  Private->PciIo->Attributes (
                    Private->PciIo,
                    EfiPciIoAttributeOperationSet,
                    Private->OriginalPciAttributes,
                    NULL
                    );

ClosePciIo:
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

FreePrivate:
  FreePool (Private);

RestoreTpl:
  gBS->RestoreTPL (OldTpl);

  return Status;
}

/**
  Stop this device

  @param  This                   The USB bus driver binding protocol.
  @param  Controller             The controller to release.
  @param  NumberOfChildren       The number of children of this device that
                                 opened the controller BY_CHILD.
  @param  ChildHandleBuffer      The array of child handle.

  @retval EFI_SUCCESS            The controller or children are stopped.
  @retval EFI_DEVICE_ERROR       Failed to stop the driver.

**/
EFI_STATUS
EFIAPI
QemuVideoControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;

  EFI_STATUS               Status;
  QEMU_VIDEO_PRIVATE_DATA  *Private;

  if (NumberOfChildren == 0) {
    //
    // Close the PCI I/O Protocol
    //
    gBS->CloseProtocol (
           Controller,
           &gEfiPciIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );
    return EFI_SUCCESS;
  }

  //
  // free all resources for whose access we need the child handle, because the
  // child handle is going away
  //
  ASSERT (NumberOfChildren == 1);
  Status = gBS->OpenProtocol (
                  ChildHandleBuffer[0],
                  mMsGopOverrideProtocolGuid, // MU_CHANGE use MsGopOverrideProtocolGuid
                  (VOID **)&GraphicsOutput,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get our private context information
  //
  Private = QEMU_VIDEO_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (GraphicsOutput);
  ASSERT (Private->Handle == ChildHandleBuffer[0]);

  QemuVideoGraphicsOutputDestructor (Private);
  //
  // Remove the GOP protocol interface from the system
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->Handle,
                  mMsGopOverrideProtocolGuid, // MU_CHANGE use MsGopOverrideProtocolGuid
                  &Private->GraphicsOutput,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Restore original PCI attributes
  //
  Private->PciIo->Attributes (
                    Private->PciIo,
                    EfiPciIoAttributeOperationSet,
                    Private->OriginalPciAttributes,
                    NULL
                    );

  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Private->Handle
         );

  FreePool (Private->ModeData);
  gBS->UninstallProtocolInterface (
         Private->Handle,
         &gEfiDevicePathProtocolGuid,
         Private->GopDevicePath
         );
  FreePool (Private->GopDevicePath);

  //
  // Free our instance data
  //
  gBS->FreePool (Private);

  return EFI_SUCCESS;
}

/**
  TODO: Add function description

  @param  Private TODO: add argument description
  @param  Address TODO: add argument description
  @param  Data TODO: add argument description

  TODO: add return values

**/
VOID
outb (
  QEMU_VIDEO_PRIVATE_DATA  *Private,
  UINTN                    Address,
  UINT8                    Data
  )
{
  Private->PciIo->Io.Write (
                       Private->PciIo,
                       EfiPciIoWidthUint8,
                       EFI_PCI_IO_PASS_THROUGH_BAR,
                       Address,
                       1,
                       &Data
                       );
}

/**
  TODO: Add function description

  @param  Private TODO: add argument description
  @param  Address TODO: add argument description
  @param  Data TODO: add argument description

  TODO: add return values

**/
VOID
outw (
  QEMU_VIDEO_PRIVATE_DATA  *Private,
  UINTN                    Address,
  UINT16                   Data
  )
{
  Private->PciIo->Io.Write (
                       Private->PciIo,
                       EfiPciIoWidthUint16,
                       EFI_PCI_IO_PASS_THROUGH_BAR,
                       Address,
                       1,
                       &Data
                       );
}

/**
  TODO: Add function description

  @param  Private TODO: add argument description
  @param  Address TODO: add argument description

  TODO: add return values

**/
UINT8
inb (
  QEMU_VIDEO_PRIVATE_DATA  *Private,
  UINTN                    Address
  )
{
  UINT8  Data;

  Private->PciIo->Io.Read (
                       Private->PciIo,
                       EfiPciIoWidthUint8,
                       EFI_PCI_IO_PASS_THROUGH_BAR,
                       Address,
                       1,
                       &Data
                       );
  return Data;
}

/**
  TODO: Add function description

  @param  Private TODO: add argument description
  @param  Address TODO: add argument description

  TODO: add return values

**/
UINT16
inw (
  QEMU_VIDEO_PRIVATE_DATA  *Private,
  UINTN                    Address
  )
{
  UINT16  Data;

  Private->PciIo->Io.Read (
                       Private->PciIo,
                       EfiPciIoWidthUint16,
                       EFI_PCI_IO_PASS_THROUGH_BAR,
                       Address,
                       1,
                       &Data
                       );
  return Data;
}

/**
  TODO: Add function description

  @param  Private TODO: add argument description
  @param  Index TODO: add argument description
  @param  Red TODO: add argument description
  @param  Green TODO: add argument description
  @param  Blue TODO: add argument description

  TODO: add return values

**/
VOID
SetPaletteColor (
  QEMU_VIDEO_PRIVATE_DATA  *Private,
  UINTN                    Index,
  UINT8                    Red,
  UINT8                    Green,
  UINT8                    Blue
  )
{
  outb (Private, PALETTE_INDEX_REGISTER, (UINT8)Index);
  outb (Private, PALETTE_DATA_REGISTER, (UINT8)(Red >> 2));
  outb (Private, PALETTE_DATA_REGISTER, (UINT8)(Green >> 2));
  outb (Private, PALETTE_DATA_REGISTER, (UINT8)(Blue >> 2));
}

/**
  TODO: Add function description

  @param  Private TODO: add argument description

  TODO: add return values

**/
VOID
SetDefaultPalette (
  QEMU_VIDEO_PRIVATE_DATA  *Private
  )
{
  UINTN  Index;
  UINTN  RedIndex;
  UINTN  GreenIndex;
  UINTN  BlueIndex;

  Index = 0;
  for (RedIndex = 0; RedIndex < 8; RedIndex++) {
    for (GreenIndex = 0; GreenIndex < 8; GreenIndex++) {
      for (BlueIndex = 0; BlueIndex < 4; BlueIndex++) {
        SetPaletteColor (Private, Index, (UINT8)(RedIndex << 5), (UINT8)(GreenIndex << 5), (UINT8)(BlueIndex << 6));
        Index++;
      }
    }
  }
}

/**
  TODO: Add function description

  @param  Private TODO: add argument description

  TODO: add return values

**/
VOID
ClearScreen (
  QEMU_VIDEO_PRIVATE_DATA  *Private
  )
{
  UINT32  Color;

  Color = 0;
  Private->PciIo->Mem.Write (
                        Private->PciIo,
                        EfiPciIoWidthFillUint32,
                        Private->FrameBufferVramBarIndex,
                        0,
                        0x400000 >> 2,
                        &Color
                        );
}

/**
  TODO: Add function description

  @param  Private TODO: add argument description

  TODO: add return values

**/
VOID
DrawLogo (
  QEMU_VIDEO_PRIVATE_DATA  *Private,
  UINTN                    ScreenWidth,
  UINTN                    ScreenHeight
  )
{
}

/**
  TODO: Add function description

  @param  Private TODO: add argument description
  @param  ModeData TODO: add argument description

  TODO: add return values

**/
VOID
InitializeCirrusGraphicsMode (
  QEMU_VIDEO_PRIVATE_DATA  *Private,
  QEMU_VIDEO_CIRRUS_MODES  *ModeData
  )
{
  UINT8  Byte;
  UINTN  Index;

  outw (Private, SEQ_ADDRESS_REGISTER, 0x1206);
  outw (Private, SEQ_ADDRESS_REGISTER, 0x0012);

  for (Index = 0; Index < 15; Index++) {
    outw (Private, SEQ_ADDRESS_REGISTER, ModeData->SeqSettings[Index]);
  }

  if (Private->Variant == QEMU_VIDEO_CIRRUS_5430) {
    outb (Private, SEQ_ADDRESS_REGISTER, 0x0f);
    Byte = (UINT8)((inb (Private, SEQ_DATA_REGISTER) & 0xc7) ^ 0x30);
    outb (Private, SEQ_DATA_REGISTER, Byte);
  }

  outb (Private, MISC_OUTPUT_REGISTER, ModeData->MiscSetting);
  outw (Private, GRAPH_ADDRESS_REGISTER, 0x0506);
  outw (Private, SEQ_ADDRESS_REGISTER, 0x0300);
  outw (Private, CRTC_ADDRESS_REGISTER, 0x2011);

  for (Index = 0; Index < 28; Index++) {
    outw (Private, CRTC_ADDRESS_REGISTER, (UINT16)((ModeData->CrtcSettings[Index] << 8) | Index));
  }

  for (Index = 0; Index < 9; Index++) {
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16)((GraphicsController[Index] << 8) | Index));
  }

  inb (Private, INPUT_STATUS_1_REGISTER);

  for (Index = 0; Index < 21; Index++) {
    outb (Private, ATT_ADDRESS_REGISTER, (UINT8)Index);
    outb (Private, ATT_ADDRESS_REGISTER, AttributeController[Index]);
  }

  outb (Private, ATT_ADDRESS_REGISTER, 0x20);

  outw (Private, GRAPH_ADDRESS_REGISTER, 0x0009);
  outw (Private, GRAPH_ADDRESS_REGISTER, 0x000a);
  outw (Private, GRAPH_ADDRESS_REGISTER, 0x000b);
  outb (Private, DAC_PIXEL_MASK_REGISTER, 0xff);

  SetDefaultPalette (Private);
  ClearScreen (Private);
}

EFI_STATUS
EFIAPI
InitializeQemuVideo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS       Status;
  UINT16           PolicySize;
  UINT64           PolicyAttribute;
  POLICY_PROTOCOL  *PolicyProtocol;

  mMsGopOverrideProtocolGuid = PcdGetPtr (PcdMsGopOverrideProtocolGuid); // MU_CHANGE use MsGopOverrideProtocolGuid

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gQemuVideoDriverBinding,
             ImageHandle,
             &gQemuVideoComponentName,
             &gQemuVideoComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gPolicyProtocolGuid,
                  NULL,
                  (VOID **)&PolicyProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: Failed to locate policy protocol - %r!\n", __FUNCTION__, Status));
    return Status;
  }

  PolicySize = sizeof (mGfxPolicy);
  Status     = PolicyProtocol->GetPolicy (
                                 &gPolicyDataGFXGuid,
                                 &PolicyAttribute,
                                 mGfxPolicy,
                                 &PolicySize
                                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "%a: Failed to get GFX policy from database, configuration is not setup properly - %r! Default to disabled state.\n", __FUNCTION__, Status));
    mGfxPolicy[0].Power_State_Port = FALSE;
    // don't return a failed status in this case
    PolicySize = sizeof (mGfxPolicy);
    Status     = EFI_SUCCESS;
  }

  if (PolicySize != sizeof (mGfxPolicy)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Located USB policy is not valid! Attributes: 0x%llx, has size: %x, expecting %x.\n",
      __FUNCTION__,
      PolicyAttribute,
      PolicySize,
      sizeof (mGfxPolicy)
      ));
    return EFI_COMPROMISED_DATA;
  }

  if ((PolicyAttribute & POLICY_ATTRIBUTE_FINALIZED) == 0) {
    DEBUG ((DEBUG_WARN, "%a: Applying platform default configuration (Attribute: %llx)!\n", __FUNCTION__, PolicyAttribute));
  }

  return Status;
}
