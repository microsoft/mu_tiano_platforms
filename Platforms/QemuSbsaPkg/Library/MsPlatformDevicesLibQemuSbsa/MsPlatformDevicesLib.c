/** @file
 *MsPlatformDevicesLib  - Device specific library.

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Virtio095.h>

#include <Guid/DebugAgentGuid.h>
#include <Guid/QemuRamfb.h>
#include <Guid/SerialPortLibVendor.h>

#include <Protocol/DevicePath.h>
#include <Protocol/PciIo.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DeviceBootManagerLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MsPlatformDevicesLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/XenPlatformLib.h>

#include <OvmfPlatforms.h>

//
// Global data
//

UINT16  mHostBridgeDevId;

//
// Table of host IRQs matching PCI IRQs A-D
// (for configuring PCI Interrupt Line register)
//
CONST UINT8  PciHostIrqs[] = {
  0x0a, 0x0a, 0x0b, 0x0b
};

#define PCI_DEVICE_PATH_NODE(Func, Dev) \
  { \
    { \
      HARDWARE_DEVICE_PATH, \
      HW_PCI_DP, \
      { \
        (UINT8) (sizeof (PCI_DEVICE_PATH)), \
        (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8) \
      } \
    }, \
    (Func), \
    (Dev) \
  }

#define PNPID_DEVICE_PATH_NODE(PnpId) \
  { \
    { \
      ACPI_DEVICE_PATH, \
      ACPI_DP, \
      { \
        (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)), \
        (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8) \
      }, \
    }, \
    EISA_PNP_ID((PnpId)), \
    0 \
  }

#define gPciIsaBridge \
  PCI_DEVICE_PATH_NODE(0, 0x1f)

#define gP2PBridge \
  PCI_DEVICE_PATH_NODE(0, 0x1e)

#define gPnpPs2Keyboard \
  PNPID_DEVICE_PATH_NODE(0x0303)

#define gPnp16550ComPort \
  PNPID_DEVICE_PATH_NODE(0x0501)

#define gUart \
  { \
    { \
      MESSAGING_DEVICE_PATH, \
      MSG_UART_DP, \
      { \
        (UINT8) (sizeof (UART_DEVICE_PATH)), \
        (UINT8) ((sizeof (UART_DEVICE_PATH)) >> 8) \
      } \
    }, \
    0, \
    115200, \
    8, \
    1, \
    1 \
  }

#define gPcAnsiTerminal \
  { \
    { \
      MESSAGING_DEVICE_PATH, \
      MSG_VENDOR_DP, \
      { \
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)), \
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8) \
      } \
    }, \
    DEVICE_PATH_MESSAGING_PC_ANSI \
  }

#define gPciRootBridge \
  { \
    {\
      ACPI_DEVICE_PATH, \
      ACPI_DP, \
      {\
        (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)), \
        (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8), \
      },\
    },\
    EISA_PNP_ID (0x0A03), \
    0 \
  }

#define gEndEntire \
  { \
    END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, { END_DEVICE_PATH_LENGTH, 0 } \
  }

#define PCI_CLASS_SCC        0x07
#define PCI_SUBCLASS_SERIAL  0x00
#define PCI_IF_16550         0x02
#define IS_PCI_16550SERIAL(_p)  IS_CLASS3 (_p, PCI_CLASS_SCC, PCI_SUBCLASS_SERIAL, PCI_IF_16550)
#define IS_PCI_ISA_PDECODE(_p)  IS_CLASS3 (_p, PCI_CLASS_BRIDGE, PCI_CLASS_BRIDGE_ISA_PDECODE, 0)

//
// Vendor UART Device Path structure
//
#pragma pack (1)
typedef struct {
  VENDOR_DEVICE_PATH          VendorHardware;
  UART_DEVICE_PATH            Uart;
  VENDOR_DEVICE_PATH          TerminalType;
  EFI_DEVICE_PATH_PROTOCOL    End;
} VENDOR_UART_DEVICE_PATH;
#pragma pack ()
//
// QemuRamfb Device Path structure
//
#pragma pack (1)
typedef struct {
  VENDOR_DEVICE_PATH          Vendor;
  ACPI_ADR_DEVICE_PATH        AcpiAdr;
  EFI_DEVICE_PATH_PROTOCOL    End;
} VENDOR_RAMFB_DEVICE_PATH;
#pragma pack ()

//
// USB Keyboard Device Path structure
//
#pragma pack (1)
typedef struct {
  USB_CLASS_DEVICE_PATH       Keyboard;
  EFI_DEVICE_PATH_PROTOCOL    End;
} USB_KEYBOARD_DEVICE_PATH;
#pragma pack ()

ACPI_HID_DEVICE_PATH  gPnpPs2KeyboardDeviceNode  = gPnpPs2Keyboard;
ACPI_HID_DEVICE_PATH  gPnp16550ComPortDeviceNode = gPnp16550ComPort;
UART_DEVICE_PATH      gUartDeviceNode            = gUart;
VENDOR_DEVICE_PATH    gTerminalTypeDeviceNode    = gPcAnsiTerminal;

//
// Debug Agent UART Device Path
//
VENDOR_UART_DEVICE_PATH  gDebugAgentUartDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8)(sizeof (VENDOR_DEVICE_PATH)),
        (UINT8)((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    EFI_DEBUG_AGENT_GUID,
  },
  {
    {
      MESSAGING_DEVICE_PATH,
      MSG_UART_DP,
      {
        (UINT8)(sizeof (UART_DEVICE_PATH)),
        (UINT8)((sizeof (UART_DEVICE_PATH)) >> 8)
      }
    },
    0,  // Reserved
    0,  // BaudRate - Default
    0,  // DataBits - Default
    0,  // Parity   - Default
    0,  // StopBits - Default
  },
  gPcAnsiTerminal,
  gEndEntire
};

STATIC USB_KEYBOARD_DEVICE_PATH  gUsbKeyboardDevicePath = {
  {
    {
      MESSAGING_DEVICE_PATH,
      MSG_USB_CLASS_DP,
      {
        (UINT8)sizeof (USB_CLASS_DEVICE_PATH),
        (UINT8)(sizeof (USB_CLASS_DEVICE_PATH) >> 8)
      }
    },
    0xFFFF, // VendorId: any
    0xFFFF, // ProductId: any
    3,      // DeviceClass: HID
    1,      // DeviceSubClass: boot
    1       // DeviceProtocol: keyboard
  },
  gEndEntire
};

typedef struct {
  ACPI_HID_DEVICE_PATH        PciRootBridge;
  PCI_DEVICE_PATH             PciDevice;
  EFI_DEVICE_PATH_PROTOCOL    End;
} PREFERRED_VIDEO_DEVICE;

STATIC PREFERRED_VIDEO_DEVICE  gPreferredVideo = {
  gPciRootBridge,
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_PCI_DP,
      {
        (UINT8)(sizeof (PCI_DEVICE_PATH)),
        (UINT8)((sizeof (PCI_DEVICE_PATH)) >> 8),
      },
    },
    0x00,
    0x01
  },
  gEndEntire
};

//
// Predefined platform default console device path
//
BDS_CONSOLE_CONNECT_ENTRY  gPlatformConsoles[] = {
  {
    (EFI_DEVICE_PATH_PROTOCOL *)&gDebugAgentUartDevicePath,
    (CONSOLE_OUT | CONSOLE_IN | STD_ERROR)
  },
  {
    (EFI_DEVICE_PATH_PROTOCOL *)&gUsbKeyboardDevicePath,
    CONSOLE_IN
  },
  {
    (EFI_DEVICE_PATH_PROTOCOL *)&gPreferredVideo,
    CONSOLE_OUT
  },
  {
    NULL,
    0
  }
};

static EFI_DEVICE_PATH_PROTOCOL  *gPlatformConInDeviceList[] = {
  NULL
};

typedef
EFI_STATUS
(EFIAPI *PROTOCOL_INSTANCE_CALLBACK)(
  IN EFI_HANDLE           Handle,
  IN VOID                 *Instance,
  IN VOID                 *Context
  );

/**
  @param[in]  Handle - Handle of PCI device instance
  @param[in]  PciIo - PCI IO protocol instance
  @param[in]  Pci - PCI Header register block
**/
typedef
EFI_STATUS
(EFIAPI *VISIT_PCI_INSTANCE_CALLBACK)(
  IN EFI_HANDLE           Handle,
  IN EFI_PCI_IO_PROTOCOL  *PciIo,
  IN PCI_TYPE00           *Pci
  );

EFI_STATUS
GetGopDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL  *PciDevicePath,
  OUT EFI_DEVICE_PATH_PROTOCOL  **GopDevicePath
  )
{
  UINTN                     Index;
  EFI_STATUS                Status;
  EFI_HANDLE                PciDeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempPciDevicePath;
  UINTN                     GopHandleCount;
  EFI_HANDLE                *GopHandleBuffer;

  if ((PciDevicePath == NULL) || (GopDevicePath == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the GopDevicePath to be PciDevicePath
  //
  *GopDevicePath    = PciDevicePath;
  TempPciDevicePath = PciDevicePath;

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &TempPciDevicePath,
                  &PciDeviceHandle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Try to connect this handle, so that GOP driver could start on this
  // device and create child handles with GraphicsOutput Protocol installed
  // on them, then we get device paths of these child handles and select
  // them as possible console device.
  //
  gBS->ConnectController (PciDeviceHandle, NULL, NULL, FALSE);

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &GopHandleCount,
                  &GopHandleBuffer
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Add all the child handles as possible Console Device
    //
    for (Index = 0; Index < GopHandleCount; Index++) {
      Status = gBS->HandleProtocol (
                      GopHandleBuffer[Index],
                      &gEfiDevicePathProtocolGuid,
                      (VOID *)&TempDevicePath
                      );
      if (EFI_ERROR (Status)) {
        continue;
      }

      if (CompareMem (
            PciDevicePath,
            TempDevicePath,
            GetDevicePathSize (PciDevicePath) - END_DEVICE_PATH_LENGTH
            ) == 0)
      {
        //
        // In current implementation, we only enable one of the child handles
        // as console device, i.e. set one of the child handle's device
        // path to variable "ConOut"
        // In future, we could select all child handles to be console device
        //

        *GopDevicePath = TempDevicePath;

        //
        // Delete the PCI device's path that added by
        // GetPlugInPciVgaDevicePath(). Add the integrity GOP device path.
        //
        EfiBootManagerUpdateConsoleVariable (ConOutDev, NULL, PciDevicePath);
        EfiBootManagerUpdateConsoleVariable (ConOutDev, TempDevicePath, NULL);
      }
    }

    gBS->FreePool (GopHandleBuffer);
  }

  return EFI_SUCCESS;
}

/**
  Add PCI display to ConOut.

  @param[in] DeviceHandle  Handle of the PCI display device.

  @retval EFI_SUCCESS  The PCI display device has been added to ConOut.

  @return              Error codes, due to EFI_DEVICE_PATH_PROTOCOL missing
                       from DeviceHandle.
**/
EFI_STATUS
PreparePciDisplayDevicePath (
  IN EFI_HANDLE  DeviceHandle
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *GopDevicePath;

  DevicePath    = NULL;
  GopDevicePath = NULL;
  Status        = gBS->HandleProtocol (
                         DeviceHandle,
                         &gEfiDevicePathProtocolGuid,
                         (VOID *)&DevicePath
                         );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GetGopDevicePath (DevicePath, &GopDevicePath);
  DevicePath = GopDevicePath;

  EfiBootManagerUpdateConsoleVariable (ConOut, DevicePath, NULL);

  return EFI_SUCCESS;
}

/**
  Add PCI Serial to ConOut, ConIn, ErrOut.

  @param[in] DeviceHandle  Handle of the PCI serial device.

  @retval EFI_SUCCESS  The PCI serial device has been added to ConOut, ConIn,
                       ErrOut.

  @return              Error codes, due to EFI_DEVICE_PATH_PROTOCOL missing
                       from DeviceHandle.
**/
EFI_STATUS
PreparePciSerialDevicePath (
  IN EFI_HANDLE  DeviceHandle
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  DevicePath = NULL;
  Status     = gBS->HandleProtocol (
                      DeviceHandle,
                      &gEfiDevicePathProtocolGuid,
                      (VOID *)&DevicePath
                      );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode
                 );
  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode
                 );

  EfiBootManagerUpdateConsoleVariable (ConOut, DevicePath, NULL);
  EfiBootManagerUpdateConsoleVariable (ConIn, DevicePath, NULL);
  EfiBootManagerUpdateConsoleVariable (ErrOut, DevicePath, NULL);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
VisitAllInstancesOfProtocol (
  IN EFI_GUID                    *Id,
  IN PROTOCOL_INSTANCE_CALLBACK  CallBackFunction,
  IN VOID                        *Context
  )
{
  EFI_STATUS  Status;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINTN       Index;
  VOID        *Instance;

  //
  // Start to check all the PciIo to find all possible device
  //
  HandleCount  = 0;
  HandleBuffer = NULL;
  Status       = gBS->LocateHandleBuffer (
                        ByProtocol,
                        Id,
                        NULL,
                        &HandleCount,
                        &HandleBuffer
                        );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], Id, &Instance);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = (*CallBackFunction)(
  HandleBuffer[Index],
  Instance,
  Context
  );
  }

  gBS->FreePool (HandleBuffer);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VisitingAPciInstance (
  IN EFI_HANDLE  Handle,
  IN VOID        *Instance,
  IN VOID        *Context
  )
{
  EFI_STATUS           Status;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  PCI_TYPE00           Pci;

  PciIo = (EFI_PCI_IO_PROTOCOL *)Instance;

  //
  // Check for all PCI device
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return (*(VISIT_PCI_INSTANCE_CALLBACK)(UINTN)Context)(
  Handle,
  PciIo,
  &Pci
  );
}

EFI_STATUS
VisitAllPciInstances (
  IN VISIT_PCI_INSTANCE_CALLBACK  CallBackFunction
  )
{
  return VisitAllInstancesOfProtocol (
           &gEfiPciIoProtocolGuid,
           VisitingAPciInstance,
           (VOID *)(UINTN)CallBackFunction
           );
}

STATIC
EFI_STATUS
EFIAPI
ConnectVirtioPciRng (
  IN EFI_HANDLE  Handle,
  IN VOID        *Instance,
  IN VOID        *Context
  )
{
  EFI_PCI_IO_PROTOCOL  *PciIo;
  EFI_STATUS           Status;
  UINT16               VendorId;
  UINT16               DeviceId;
  UINT8                RevisionId;
  BOOLEAN              Virtio10;
  UINT16               SubsystemId;

  PciIo = Instance;

  //
  // Read and check VendorId.
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint16,
                        PCI_VENDOR_ID_OFFSET,
                        1,
                        &VendorId
                        );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  if (VendorId != VIRTIO_VENDOR_ID) {
    return EFI_SUCCESS;
  }

  //
  // Read DeviceId and RevisionId.
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint16,
                        PCI_DEVICE_ID_OFFSET,
                        1,
                        &DeviceId
                        );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_REVISION_ID_OFFSET,
                        1,
                        &RevisionId
                        );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // From DeviceId and RevisionId, determine whether the device is a
  // modern-only Virtio 1.0 device. In case of Virtio 1.0, DeviceId can
  // immediately be restricted to VIRTIO_SUBSYSTEM_ENTROPY_SOURCE, and
  // SubsystemId will only play a sanity-check role. Otherwise, DeviceId can
  // only be sanity-checked, and SubsystemId will decide.
  //
  if ((DeviceId == 0x1040 + VIRTIO_SUBSYSTEM_ENTROPY_SOURCE) &&
      (RevisionId >= 0x01))
  {
    Virtio10 = TRUE;
  } else if ((DeviceId >= 0x1000) && (DeviceId <= 0x103F) && (RevisionId == 0x00)) {
    Virtio10 = FALSE;
  } else {
    return EFI_SUCCESS;
  }

  //
  // Read and check SubsystemId as dictated by Virtio10.
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint16,
                        PCI_SUBSYSTEM_ID_OFFSET,
                        1,
                        &SubsystemId
                        );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  if ((Virtio10 && (SubsystemId >= 0x40)) ||
      (!Virtio10 && (SubsystemId == VIRTIO_SUBSYSTEM_ENTROPY_SOURCE)))
  {
    Status = gBS->ConnectController (
                    Handle, // ControllerHandle
                    NULL,   // DriverImageHandle -- connect all drivers
                    NULL,   // RemainingDevicePath -- produce all child handles
                    FALSE   // Recursive -- don't follow child handles
                    );
    if (EFI_ERROR (Status)) {
      goto Error;
    }
  }

  return EFI_SUCCESS;

Error:
  DEBUG ((DEBUG_ERROR, "%a: %r\n", __FUNCTION__, Status));
  return Status;
}

/**
  Add IsaKeyboard to ConIn; add IsaSerial to ConOut, ConIn, ErrOut.

  @param[in] DeviceHandle  Handle of the LPC Bridge device.

  @retval EFI_SUCCESS  Console devices on the LPC bridge have been added to
                       ConOut, ConIn, and ErrOut.

  @return              Error codes, due to EFI_DEVICE_PATH_PROTOCOL missing
                       from DeviceHandle.
**/
EFI_STATUS
PrepareLpcBridgeDevicePath (
  IN EFI_HANDLE  DeviceHandle
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  CHAR16                    *DevPathStr;

  DevicePath = NULL;
  Status     = gBS->HandleProtocol (
                      DeviceHandle,
                      &gEfiDevicePathProtocolGuid,
                      (VOID *)&DevicePath
                      );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TempDevicePath = DevicePath;

  //
  // Register Keyboard
  //
  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&gPnpPs2KeyboardDeviceNode
                 );

  EfiBootManagerUpdateConsoleVariable (ConIn, DevicePath, NULL);

  //
  // Register COM1
  //
  DevicePath                     = TempDevicePath;
  gPnp16550ComPortDeviceNode.UID = 0;

  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&gPnp16550ComPortDeviceNode
                 );
  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode
                 );
  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode
                 );

  //
  // Print Device Path
  //
  DevPathStr = ConvertDevicePathToText (DevicePath, FALSE, FALSE);
  if (DevPathStr != NULL) {
    DEBUG ((
      DEBUG_INFO,
      "BdsPlatform.c+%d: COM%d DevPath: %s\n",
      __LINE__,
      gPnp16550ComPortDeviceNode.UID + 1,
      DevPathStr
      ));
    gBS->FreePool (DevPathStr);
  }

  EfiBootManagerUpdateConsoleVariable (ConOut, DevicePath, NULL);
  EfiBootManagerUpdateConsoleVariable (ConIn, DevicePath, NULL);
  EfiBootManagerUpdateConsoleVariable (ErrOut, DevicePath, NULL);

  //
  // Register COM2
  //
  DevicePath                     = TempDevicePath;
  gPnp16550ComPortDeviceNode.UID = 1;

  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&gPnp16550ComPortDeviceNode
                 );
  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode
                 );
  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode
                 );

  //
  // Print Device Path
  //
  DevPathStr = ConvertDevicePathToText (DevicePath, FALSE, FALSE);
  if (DevPathStr != NULL) {
    DEBUG ((
      DEBUG_INFO,
      "BdsPlatform.c+%d: COM%d DevPath: %s\n",
      __LINE__,
      gPnp16550ComPortDeviceNode.UID + 1,
      DevPathStr
      ));
    gBS->FreePool (DevPathStr);
  }

  EfiBootManagerUpdateConsoleVariable (ConOut, DevicePath, NULL);
  EfiBootManagerUpdateConsoleVariable (ConIn, DevicePath, NULL);
  EfiBootManagerUpdateConsoleVariable (ErrOut, DevicePath, NULL);

  return EFI_SUCCESS;
}

/**
  Configure PCI Interrupt Line register for applicable devices
  Ported from SeaBIOS, src/fw/pciinit.c, *_pci_slot_get_irq()

  @param[in]  Handle - Handle of PCI device instance
  @param[in]  PciIo - PCI IO protocol instance
  @param[in]  PciHdr - PCI Header register block

  @retval EFI_SUCCESS - PCI Interrupt Line register configured successfully.

**/
EFI_STATUS
EFIAPI
SetPciIntLine (
  IN EFI_HANDLE           Handle,
  IN EFI_PCI_IO_PROTOCOL  *PciIo,
  IN PCI_TYPE00           *PciHdr
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *DevPathNode;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  UINTN                     RootSlot;
  UINTN                     Idx;
  UINT8                     IrqLine;
  EFI_STATUS                Status;
  UINT32                    RootBusNumber;

  Status = EFI_SUCCESS;

  if (PciHdr->Device.InterruptPin != 0) {
    DevPathNode = DevicePathFromHandle (Handle);
    ASSERT (DevPathNode != NULL);
    DevPath = DevPathNode;

    RootBusNumber = 0;
    if ((DevicePathType (DevPathNode) == ACPI_DEVICE_PATH) &&
        (DevicePathSubType (DevPathNode) == ACPI_DP) &&
        (((ACPI_HID_DEVICE_PATH *)DevPathNode)->HID == EISA_PNP_ID (0x0A03)))
    {
      RootBusNumber = ((ACPI_HID_DEVICE_PATH *)DevPathNode)->UID;
    }

    //
    // Compute index into PciHostIrqs[] table by walking
    // the device path and adding up all device numbers
    //
    Status   = EFI_NOT_FOUND;
    RootSlot = 0;
    Idx      = PciHdr->Device.InterruptPin - 1;
    while (!IsDevicePathEnd (DevPathNode)) {
      if ((DevicePathType (DevPathNode) == HARDWARE_DEVICE_PATH) &&
          (DevicePathSubType (DevPathNode) == HW_PCI_DP))
      {
        Idx += ((PCI_DEVICE_PATH *)DevPathNode)->Device;

        //
        // Unlike SeaBIOS, which starts climbing from the leaf device
        // up toward the root, we traverse the device path starting at
        // the root moving toward the leaf node.
        // The slot number of the top-level parent bridge is needed for
        // Q35 cases with more than 24 slots on the root bus.
        //
        if (Status != EFI_SUCCESS) {
          Status   = EFI_SUCCESS;
          RootSlot = ((PCI_DEVICE_PATH *)DevPathNode)->Device;
        }
      }

      DevPathNode = NextDevicePathNode (DevPathNode);
    }

    if (EFI_ERROR (Status)) {
      return Status;
    }

    if ((RootBusNumber == 0) && (RootSlot == 0)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: PCI host bridge (00:00.0) should have no interrupts!\n",
        __FUNCTION__
        ));
      ASSERT (FALSE);
    }

    //
    // Final PciHostIrqs[] index calculation depends on the platform
    // and should match SeaBIOS src/fw/pciinit.c *_pci_slot_get_irq()
    //
    switch (mHostBridgeDevId) {
      case INTEL_82441_DEVICE_ID:
        Idx -= 1;
        break;
      case INTEL_Q35_MCH_DEVICE_ID:
        //
        // SeaBIOS contains the following comment:
        // "Slots 0-24 rotate slot:pin mapping similar to piix above, but
        //  with a different starting index - see q35-acpi-dsdt.dsl.
        //
        //  Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H)"
        //
        if (RootSlot > 24) {
          //
          // in this case, subtract back out RootSlot from Idx
          // (SeaBIOS never adds it to begin with, but that would make our
          //  device path traversal loop above too awkward)
          //
          Idx -= RootSlot;
        }

        break;
      default:
        ASSERT (FALSE); // should never get here
    }

    Idx    %= ARRAY_SIZE (PciHostIrqs);
    IrqLine = PciHostIrqs[Idx];

    DEBUG_CODE_BEGIN ();
    {
      CHAR16         *DevPathString;
      STATIC CHAR16  Fallback[] = L"<failed to convert>";
      UINTN          Segment, Bus, Device, Function;

      DevPathString = ConvertDevicePathToText (DevPath, FALSE, FALSE);
      if (DevPathString == NULL) {
        DevPathString = Fallback;
      }

      Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
      ASSERT_EFI_ERROR (Status);

      DEBUG ((
        DEBUG_VERBOSE,
        "%a: [%02x:%02x.%x] %s -> 0x%02x\n",
        __FUNCTION__,
        (UINT32)Bus,
        (UINT32)Device,
        (UINT32)Function,
        DevPathString,
        IrqLine
        ));

      if (DevPathString != Fallback) {
        gBS->FreePool (DevPathString);
      }
    }
    DEBUG_CODE_END ();

    //
    // Set PCI Interrupt Line register for this device to PciHostIrqs[Idx]
    //
    Status = PciIo->Pci.Write (
                          PciIo,
                          EfiPciIoWidthUint8,
                          PCI_INT_LINE_OFFSET,
                          1,
                          &IrqLine
                          );
  }

  return Status;
}

VOID
PciAcpiInitialization (
  )
{
  UINTN  Pmba;

  //
  // Query Host Bridge DID to determine platform type
  //
  mHostBridgeDevId = PcdGet16 (PcdOvmfHostBridgePciDevId);
  switch (mHostBridgeDevId) {
    case INTEL_82441_DEVICE_ID:
      Pmba = POWER_MGMT_REGISTER_PIIX4 (PIIX4_PMBA);
      //
      // 00:01.0 ISA Bridge (PIIX4) LNK routing targets
      //
      PciWrite8 (PCI_LIB_ADDRESS (0, 1, 0, 0x60), 0x0b); // A
      PciWrite8 (PCI_LIB_ADDRESS (0, 1, 0, 0x61), 0x0b); // B
      PciWrite8 (PCI_LIB_ADDRESS (0, 1, 0, 0x62), 0x0a); // C
      PciWrite8 (PCI_LIB_ADDRESS (0, 1, 0, 0x63), 0x0a); // D
      break;
    case INTEL_Q35_MCH_DEVICE_ID:
      Pmba = POWER_MGMT_REGISTER_Q35 (ICH9_PMBASE);
      //
      // 00:1f.0 LPC Bridge (Q35) LNK routing targets
      //
      PciWrite8 (PCI_LIB_ADDRESS (0, 0x1f, 0, 0x60), 0x0a); // A
      PciWrite8 (PCI_LIB_ADDRESS (0, 0x1f, 0, 0x61), 0x0a); // B
      PciWrite8 (PCI_LIB_ADDRESS (0, 0x1f, 0, 0x62), 0x0b); // C
      PciWrite8 (PCI_LIB_ADDRESS (0, 0x1f, 0, 0x63), 0x0b); // D
      PciWrite8 (PCI_LIB_ADDRESS (0, 0x1f, 0, 0x68), 0x0a); // E
      PciWrite8 (PCI_LIB_ADDRESS (0, 0x1f, 0, 0x69), 0x0a); // F
      PciWrite8 (PCI_LIB_ADDRESS (0, 0x1f, 0, 0x6a), 0x0b); // G
      PciWrite8 (PCI_LIB_ADDRESS (0, 0x1f, 0, 0x6b), 0x0b); // H
      break;
    default:
      if (XenDetected ()) {
        //
        // There is no PCI bus in this case.
        //
        return;
      }

      DEBUG ((
        DEBUG_ERROR,
        "%a: Unknown Host Bridge Device ID: 0x%04x\n",
        __FUNCTION__,
        mHostBridgeDevId
        ));
      ASSERT (FALSE);
      return;
  }

  //
  // Initialize PCI_INTERRUPT_LINE for applicable present PCI devices
  //
  VisitAllPciInstances (SetPciIntLine);

  //
  // Set ACPI SCI_EN bit in PMCNTRL
  //
  IoOr16 ((PciRead32 (Pmba) & ~BIT0) + 4, BIT0);
}

/**
  Do platform specific PCI Device check and add them to
  ConOut, ConIn, ErrOut.

  @param[in]  Handle - Handle of PCI device instance
  @param[in]  PciIo - PCI IO protocol instance
  @param[in]  Pci - PCI Header register block

  @retval EFI_SUCCESS - PCI Device check and Console variable update
                        successfully.
  @retval EFI_STATUS - PCI Device check or Console variable update fail.

**/
EFI_STATUS
EFIAPI
DetectAndPreparePlatformPciDevicePath (
  IN EFI_HANDLE           Handle,
  IN EFI_PCI_IO_PROTOCOL  *PciIo,
  IN PCI_TYPE00           *Pci
  )
{
  EFI_STATUS  Status;

  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationEnable,
                    EFI_PCI_DEVICE_ENABLE,
                    NULL
                    );
  ASSERT_EFI_ERROR (Status);

  //
  // Here we decide whether it is LPC Bridge
  //
  if ((IS_PCI_LPC (Pci)) ||
      ((IS_PCI_ISA_PDECODE (Pci)) &&
       (Pci->Hdr.VendorId == 0x8086) &&
       (Pci->Hdr.DeviceId == 0x7000)
      )
      )
  {
    //
    // Add IsaKeyboard to ConIn,
    // add IsaSerial to ConOut, ConIn, ErrOut
    //
    DEBUG ((DEBUG_INFO, "Found LPC Bridge device\n"));
    PrepareLpcBridgeDevicePath (Handle);
    return EFI_SUCCESS;
  }

  //
  // Here we decide which Serial device to enable in PCI bus
  //
  if (IS_PCI_16550SERIAL (Pci)) {
    //
    // Add them to ConOut, ConIn, ErrOut.
    //
    DEBUG ((DEBUG_INFO, "Found PCI 16550 SERIAL device\n"));
    PreparePciSerialDevicePath (Handle);
    return EFI_SUCCESS;
  }

  //
  // Here we decide which display device to enable in PCI bus
  //
  if (IS_PCI_DISPLAY (Pci)) {
    //
    // Add them to ConOut.
    //
    DEBUG ((DEBUG_INFO, "Found PCI display device\n"));
    PreparePciDisplayDevicePath (Handle);
    return EFI_SUCCESS;
  }

  return Status;
}

/**
Library function used to provide the platform SD Card device path
**/
EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
GetSdCardDevicePath (
  VOID
  )
{
  return NULL;
}

/**
  Library function used to determine if the DevicePath is a valid bootable 'USB' device.
  USB here indicates the port connection type not the device protocol.
  With TBT or USB4 support PCIe storage devices are valid 'USB' boot options.
**/
BOOLEAN
EFIAPI
PlatformIsDevicePathUsb (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  return FALSE;
}

/**
Library function used to provide the list of platform devices that MUST be
connected at the beginning of BDS
**/
EFI_DEVICE_PATH_PROTOCOL **
EFIAPI
GetPlatformConnectList (
  VOID
  )
{
  //
  // Set PCI Interrupt Line registers and ACPI SCI_EN
  //
  // PciAcpiInitialization ();

  // Since this is the best place to do this, let's connect the VirtioRng
  // This is in BeforeConsole in Qemu, and this is function that's called in Q35 AfterConsole
  // So there might be some slight timing differences between when Rng comes up on Ovmf vs Q35
  // From Platforms/OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c:433 (as of 202005 or a47822d46)
  // Install both VIRTIO_DEVICE_PROTOCOL and (dependent) EFI_RNG_PROTOCOL
  // instances on Virtio PCI RNG devices.
  //
  VisitAllInstancesOfProtocol (&gEfiPciIoProtocolGuid, ConnectVirtioPciRng, NULL);

  return NULL;
}

/**
 * Library function used to provide the list of platform console devices.
 */
BDS_CONSOLE_CONNECT_ENTRY *
EFIAPI
GetPlatformConsoleList (
  VOID
  )
{
  return (BDS_CONSOLE_CONNECT_ENTRY *)&gPlatformConsoles;
}

/**
Library function used to provide the list of platform devices that MUST be connected
to support ConsoleIn activity.  This call occurs on the ConIn connect event, and
allows platforms to do specific enablement for ConsoleIn support.
**/
EFI_DEVICE_PATH_PROTOCOL **
EFIAPI
GetPlatformConnectOnConInList (
  VOID
  )
{
  return (EFI_DEVICE_PATH_PROTOCOL **)&gPlatformConInDeviceList;
}

static
BOOLEAN
IsVgaHandle (
  IN EFI_HANDLE  Handle
  )
{
  EFI_PCI_IO_PROTOCOL  *PciIo;
  PCI_TYPE00           Pci;
  EFI_STATUS           Status;

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo
                  );

  if (!EFI_ERROR (Status)) {
    Status = PciIo->Pci.Read (
                          PciIo,
                          EfiPciIoWidthUint32,
                          0,
                          sizeof (Pci) / sizeof (UINT32),
                          &Pci
                          );

    if (!EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "  PCI CLASS CODE    = 0x%x\n", Pci.Hdr.ClassCode[2]));
      DEBUG ((DEBUG_INFO, "  PCI SUBCLASS CODE = 0x%x\n", Pci.Hdr.ClassCode[1]));

      if (IS_PCI_VGA (&Pci) || IS_PCI_OLD_VGA (&Pci)) {
        DEBUG ((DEBUG_INFO, "  \nPCI VGA Device Found\n"));
        return TRUE;
      }
    }
  }

  return FALSE;
}

/**
Library function used to provide the console type.  For ConType == DisplayPath,
device path is filled in to the exact controller to use.  For other ConTypes, DisplayPath
must NULL. The device path must NOT be freed.
**/
EFI_HANDLE
EFIAPI
GetPlatformPreferredConsole (
  OUT EFI_DEVICE_PATH_PROTOCOL  **DevicePath
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                Handle = NULL;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;

  // For OVMF
  //
  // Signal the ACPI platform driver that it can download QEMU ACPI tables.
  //
  EfiEventGroupSignal (&gRootBridgesConnectedEventGroupGuid);

  // OVMF has a ton of stuff to do to make is consoles work.  Start here

  VisitAllPciInstances (DetectAndPreparePlatformPciDevicePath);

  TempDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)&gPreferredVideo;

  Status = gBS->LocateDevicePath (&gEfiPciIoProtocolGuid, &TempDevicePath, &Handle);
  if (!EFI_ERROR (Status) && IsDevicePathEnd (TempDevicePath) && IsVgaHandle (Handle)) {
  } else {
    DEBUG ((DEBUG_ERROR, "%a - Unable to locate platform preferred console. Code=%r\n", __FUNCTION__, Status));
    Status = EFI_DEVICE_ERROR;
  }

  if (Handle != NULL) {
    //
    // Connect the GOP driver
    //
    gBS->ConnectController (Handle, NULL, NULL, TRUE);

    //
    // Get the GOP device path
    // NOTE: We may get a device path that contains Controller node in it.
    //
    TempDevicePath = EfiBootManagerGetGopDevicePath (Handle);
    *DevicePath    = TempDevicePath;
  }

  return Handle;
}
