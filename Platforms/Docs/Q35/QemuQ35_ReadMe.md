# QemuQ35Pkg

**QemuQ35Pkg...**

- Is a derivative of OvmfPkg.
- Will not support Legacy BIOS or CSM.
- WIll not support S3 sleep functionality.
- Has a 32-bit PEI phase and a 64-bit DXE phase.
- Seeks to enable a tightly constrained virtual platform based on the QEMU Q35 machine type.

By solely focusing on the Q35 chipset, this package can be optimized such that it is allowed to break compatibility
with other QEMU supported chipsets. The Q35 chipset can be paired with an IA32 or X64 processor to enable a machine
that can emulate PC class hardware with industry standard features like SMM and PCI-E.

## Table of Contents

- [QemuQ35Pkg](#qemuq35pkg)
  - [Q35 Platform](#q35-platform)
  - [Compiling and Running QEMU](#compiling-and-running-qemu)
  - [Firmware Features](#firmware-features)
    - [CodeQL](#codeql)
    - [Color Bar](#color-bar)
    - [Config](#config)
    - [Device Firmware Configuration Interface (DFCI)](#device-firmware-configuration-interface-dfci)
    - [Mu Front Page](#mu-front-page)
    - [Mu Telemetry / WHEA / HwErrorRecord](#mu-telemetry--whea--hwerrorrecord)
    - [Platform Runtime Mechanism (PRM)](#platform-runtime-mechanism-prm)
    - [Trusted Platform Module (TPM)](#trusted-platform-module-tpm)
    - [Trusted Platform Module (TPM) Replay](#trusted-platform-module-tpm-replay)
    - [UEFI Memory Protections](#uefi-memory-protections)
  - [Mu Customized Components](#mu-customized-components)
    - [Modules](#modules)
    - [Libraries](#libraries)

## Q35 Platform

Q35 is a machine type that QEMU emulates.
Below is a diagram from Qemu.org about the Q35 chipset which emulates a ICH9 (I/O controller hub).

![Q35 ICH9](https://wiki.qemu.org/images/4/46/QEMU-ICH9.png)

The advantages of the ICH9 over the I440FX (which is what QEMU often emulates) is that it has PCI-E instead of just PCI
as well as having an integrated AHCI controller and no ISA bus.

Visit the feature wiki detailing QEMU Q35 for more information: <https://wiki.qemu.org/Features/Q35>

## Compiling and Running QEMU

QemuQ35Pkg uses the Project Mu repositories and Edk2 PyTools for its build operations.
Specific details can be found here [Development/building.md](../Common/building.md)

## Firmware Features

QemuQ35Pkg is a great environment to demonstrate Project Mu features without any restricted or costly physical
hardware. Current QEMU Q35 platform supports the following features provided by Project Mu:

### CodeQL

CodeQL is open source and free for open-source projects. It is maintained by GitHub and naturally has excellent
integration with GitHub projects. CodeQL uses a semantic code analysis engine to discover vulnerabilities in a
number of programming languages (both compiled and interpreted).

Project Mu (and TianoCore) use CodeQL C/C++ queries to find common programming errors and security vulnerabilities in
firmware code. This platform leverages the CodeQL build plugin from Mu Basecore that makes it very easy to run CodeQL
against this platform. You simply use provide the `--codeql` argument in your normal `stuart_update` and `stuart_build`
commands.

[Details](Features/feature_codeql.md)

### Color Bar

Color bars are used to quickly convey the Device state, based upon the DeviceStateLib. Color bars are displayed
by the ColorBarDisplayDeviceStateLib.

[Details](Features/feature_colorbar.md)

### Config

Project Mu offers a UEFI configuration feature with example implementation in `QemuQ35Pkg`. Background about the
features and more details about its integration in this repo are available in the detailed readme.

[Details](Features/feature_config.md)

### Device Firmware Configuration Interface (DFCI)

The DFCI feature enables cloud management services (MDM services like Microsoft Intune) to manage some PC
bios settings **securely**.  DFCI is a foundational feature that provides a shared identity and ownership
model between the device firmware and the cloud.  Once a device is enrolled this shared identity can be used
to securely communicate across untrusted mediums (network or usb).

[Details](../Common/Features/feature_dfci.md)

### Mu Front Page

This feature enables the Project Mu OEM sample "front page".

This is a touch friendly, graphical, UEFI HII based UI application that allows basic platform and boot device
configuration.

[Details](../Common/Features/feature_frontpage.md)

### Mu Telemetry / WHEA / HwErrorRecord

The Mu Telemetry feature is an extension of the PI spec defined report status code.  The feature is
designed to collect critical (platform defined) status codes, record them into a HwErrRecord,
and then transfer them through the Microsoft WHEA pipeline.  From there an OEM can use Microsoft
provided reports to check on in market device health.  *Some work still pending completion.

[Details](../Common/Features/feature_whea.md)

### Platform Runtime Mechanism (PRM)

Platform Runtime Mechanism (PRM) introduces the capability of moving certain classes of SMM code out of SMM and into
a code module that executes within OS context. This feature adds the PRM infrastructure to the firmware that enables
loading PRM modules which in turn are exposed to the OS for invocation. To accomplish this, a set of open source
sample PRM modules are used to demonstrate the feature and show how additional modules can be added.

[Details](Features/feature_prm.md)

### Trusted Platform Module (TPM)

QEMU TPM emulation implements a TPM TIS hardware interface that follows the Trusted Computing Group's TCG PC Client
Specific TPM Interface Specification (TIS) in addition to a TPM CRB interface that follows the TCG PC Client Platform
TPM Profile (PTP) Specification. `QemuQ35Pkg` has support to include TPM drivers and connect to the software TPM
socket interface. Usage is covered in the detailed feature readme.

[Details](Features/feature_tpm.md)

### Trusted Platform Module (TPM) Replay

An OS and firmware developer feature that allows a custom crafted TPM event log to be created and replayed during boot.
Any PCRs specified in the input TPM Replay event log are exclusively extended to the PCR (any other firmware
measurements that would normally target the PCR are blocked). This feature can be useful to test a wide range of inputs
to OS and firmware features dependent on TPM measurements

[Details](Features/feature_tpm_replay.md)

### UEFI Memory Protections

UEFI Memory Protections add safety functionality such as page and pool guards, stack guard, and null pointer
detection. The settings are split between MM and DXE environments for modularity.

[Details](../Common/Features/feature_memoryprotection.md)

## Mu Customized Components

### Modules

| Modules | Link to Documentation |
| --- | --- |
| **QemuVideoDxe** | [QEMU Cirrus Video Controller](../../QemuQ35Pkg/QemuVideoDxe/ReadMe.md) |

### Libraries

| Libraries | Link to Documentation |
| --- | --- |
| **MsPlatformDevicesLib** | [MsPlatformDevicesLib](../../QemuQ35Pkg/Library/MsPlatformDevicesLibQemuQ35/ReadMe.md) |
| **PlatformDebugLibIoPort** | [PlatformDebugLibIoPort](../../QemuQ35Pkg/Library/PlatformDebugLibIoPort/ReadMe.md) |
| **PlatformThemeLib** | [PlatformThemeLib](../../../QemuPkg/Library/PlatformThemeLib/ReadMe.md) |
