# QemuArmVirtPkg

**QemuArmVirtPkg...**

- Is another derivative of OvmfPkg based on EDK2 QEMU ARM Virt machine type.
- Will not support Legacy BIOS or CSM.
- WIll not support S3 sleep functionality.
- Has 64-bit for SEC and DXE phase, and no PEI phase.
- Seeks to enable a tightly constrained virtual platform based on the QEMU ARM CPUs.

By solely focusing on the ARM chipset, this package can be optimized such that it is allowed to break compatibility
with other QEMU supported chipsets. The ARM chipset can be paired with an AARCH64 processor to enable a machine
that can emulate ARM based hardware with industry standard features like TrustZone and PCI-E.

## QEMU ARM Virt Platform

An ARM Virt machine is an ARM based machine type that QEMU emulates.

The advantages of the virtual ARM platform over the previous SBSA platforms (which is also what QEMU emulates) is that
it has better ARM based flexible configuration options for devices and more modern hardware support. In addition, the
corresponding trusted-firmware entities are more mature and feature-rich.

## Compiling and Running QEMU

QemuArmVirtPkg uses the Project Mu repositories and Edk2 PyTools for its build operations.
Specific details can be found here [Development/building.md](../Common/building.md)

## Firmware Features

QemuArmVirtPkg is a great environment to demonstrate Project Mu features without any restricted or costly physical
hardware. Current QEMU ARM Virt platform supports the following features provided by Project Mu:

### Mu Front Page

Enable the Project Mu OEM sample "front page".
This is a touch friendly, graphical, UEFI HII based UI application that
allows basic platform and boot device configuration.

[Details](../Common/Features/feature_frontpage.md)

### Device Firmware Configuration Interface

The DFCI feature enables cloud management services (MDM services like Microsoft Intune) to manage some PC
bios settings **securely**.  DFCI is a foundational feature that provides a shared identity and ownership
model between the device firmware and the cloud.  Once a device is enrolled this shared identity can be used
to securely communicate across untrusted mediums (network or usb).

[Details](../Common/Features/feature_dfci.md)

### Mu Telemetry / WHEA / HwErrorRecord

The Mu Telemetry feature is an extension of the PI spec defined report status code.  The feature is
designed to collect critical (platform defined) status codes, record them into a HwErrRecord,
and then transfer them through the Microsoft WHEA pipeline.  From there an OEM can use Microsoft
provided reports to check on in market device health.  *Some work still pending completion.

[Details](../Common/Features/feature_whea.md)

### Trusted Platform Module (TPM)

QEMU TPM emulation implements a TPM TIS hardware interface that follows the Trusted Computing Group's TCG PC Client
Specific TPM Interface Specification (TIS) in addition to a TPM CRB interface that follows the TCG PC Client Platform
TPM Profile (PTP) Specification. `QemuArmVirtPkg` has support to include TPM drivers and connect to the software TPM
socket interface. Usage is covered in the detailed feature readme.

[Details](../Common/Features/feature_tpm.md)

## Mu Customized Components

### Modules

| Modules | Link to Documentation |
| --- | --- |
| **QemuVideoDxe** | [QEMU Video Controller](../../../QemuPkg/QemuVideoDxe/ReadMe.md) |

### Libraries

| Libraries | Link to Documentation |
| --- | --- |
| **MsPlatformDevicesLib** | [MsPlatformDevicesLib](../../QemuArmVirtPkg/Library/MsPlatformDevicesLibQemuArmVirt/ReadMe.md) |
