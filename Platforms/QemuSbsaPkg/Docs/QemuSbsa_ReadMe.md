# QemuSbsaPkg

**QemuSbsaPkg...**

- Is another derivative of OvmfPkg based on EDK2 QEMU-SBSA ARM machine type.
- Will not support Legacy BIOS or CSM.
- WIll not support S3 sleep functionality.
- Has 64-bit for both PEI and DXE phase.
- Seeks to enable a tightly constrained virtual platform based on the QEMU ARM CPUs.

By solely focusing on the ARM chipset, this package can be optimized such that it is allowed to break compatibility
with other QEMU supported chipsets. The ARM chipset can be paired with an AARCH64 processor to enable a machine
that can emulate ARM based hardware with industry standard features like TrustZone and PCI-E. Although leveraging
SBSA machine type provided by QEMU, the features enabled/included in this package will not be server class platform
centric.

## QEMU-SBSA Platform

SBSA is an ARM based machine type that QEMU emulates.

The advantages of the SBSA over the virtual ARM platform (which is what QEMU often emulates) is that it has
better ARM based platform level support (ACPI, etc.) as well as having an integrated AHCI controller.

## Compiling and Running QEMU

QemuSbsaPkg uses the Project Mu repositories and Edk2 PyTools for its build operations.
Specific details can be found here [Development/building.md](Development/building.md)

## Firmware Features

QemuSbsaPkg is a great environment to demonstrate Project Mu features without any restricted or costly physical
hardware. Current QEMU SBSA platform supports the following features provided by Project Mu:

### Mu Front Page

Enable the Project Mu OEM sample "front page".
This is a touch friendly, graphical, UEFI HII based UI application that
allows basic platform and boot device configuration.

[Details](Features/feature_frontpage.md)

### Device Firmware Configuration Interface

The DFCI feature enables cloud management services (MDM services like Microsoft Intune) to manage some PC
bios settings **securely**.  DFCI is a foundational feature that provides a shared identity and ownership
model between the device firmware and the cloud.  Once a device is enrolled this shared identity can be used
to securely communicate across untrusted mediums (network or usb).

[Details](Features/feature_dfci.md)

### Mu Telemetry / WHEA / HwErrorRecord

The Mu Telemetry feature is an extension of the PI spec defined report status code.  The feature is
designed to collect critical (platform defined) status codes, record them into a HwErrRecord,
and then transfer them through the Microsoft WHEA pipeline.  From there an OEM can use Microsoft
provided reports to check on in market device health.  *Some work still pending completion.

[Details](Features/feature_whea.md)

### Platform Runtime Mechanism (PRM)

Platform Runtime Mechanism (PRM) introduces the capability of moving certain classes of SMM code out of SMM and into
a code module that executes within OS context. This feature adds the PRM infrastructure to the firmware that enables
loading PRM modules which in turn are exposed to the OS for invocation. To accomplish this, a set of open source
sample PRM modules are used to demonstrate the feature and show how additional modules can be added.

[Details](Features/feature_prm.md)

## Mu Customized Components

### Modules

| Modules | Link to Documentation |
| --- | --- |
| **QemuVideoDxe** | [QEMU Cirrus Video Controller](../QemuVideoDxe/ReadMe.md) |

### Libraries

| Libraries | Link to Documentation |
| --- | --- |
| **MsPlatformDevicesLib** | [MsPlatformDevicesLib](../Library/MsPlatformDevicesLibQemuSbsa/ReadMe.md) |
| **PlatformDebugLibIoPort** | [PlatformDebugLibIoPort](../Library/PlatformDebugLibIoPort/ReadMe.md) |
| **PlatformThemeLib** | [PlatformThemeLib](../Library/PlatformThemeLib/ReadMe.md) |
