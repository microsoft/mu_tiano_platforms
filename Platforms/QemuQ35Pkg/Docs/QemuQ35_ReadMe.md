# QemuQ35Pkg

**QemuQ35Pkg...**

- Is a derivative of Temp.
- Will not support Legacy BIOS or CSM.
- WIll not support S3 sleep functionality.
- Has a 32-bit PEI phase and a 64-bit DXE phase.
- Seeks to enable a tightly constrained virtual platform based on the QEMU Q35 machine type.

By solely focusing on the Q35 chipset, this package can be optimized such that it is allowed to break compatibility
with other QEMU supported chipsets. The Q35 chipset can be paired with an IA32 or X64 processor to enable a machine
that can emulate PC class hardware with industry standard features like SMM and PCI-E.

## Q35 Platform

Q35 is a machine type that QEMU emulates.
Below is a diagram from Qemu.org about the Q35 chipset which emulates a ICH9 (I/O controller hub).

![Q35 ICH9](https://wiki.qemu.org/images/4/46/QEMU-ICH9.png)

The advantages of the ICH9 over the I440FX (which is what QEMU often emulates) is that it has PCI-E instead of just PCI
as well as having an integrated AHCI controller and no ISA bus.

Visit the feature wiki detailing QEMU Q35 for more information: <https://wiki.qemu.org/Features/Q35>

## Compiling and Running QEMU

QemuQ35Pkg uses the Project Mu repositories and Edk2 PyTools for its build operations.
Specific details can be found here [Development/building.md](Development/building.md)

## Firmware Features

QemuQ35Pkg is a great environment to demonstrate Project Mu features without any restricted or costly physical
hardware. Current QEMU Q35 platform supports the following features provided by Project Mu:

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

### CodeQL

CodeQL is open source and free for open-source projects. It is maintained by GitHub and naturally has excellent
integration with GitHub projects. CodeQL uses a semantic code analysis engine to discover vulnerabilities in a
number of programming languages (both compiled and interpreted).

Project Mu (and TianoCore) use CodeQL C/C++ queries to find common programming errors and security vulnerabilities in
firmware code. This platform leverages the CodeQL build plugin from Mu Basecore that makes it very easy to run CodeQL
against this platform. You simply use provide the `--codeql` argument in your normal `stuart_update` and `stuart_build`
commands.

[Details](Features/feature_codeql.md)

## Mu Customized Components

### Modules

| Modules | Link to Documentation |
| --- | --- |
| **QemuVideoDxe** | [QEMU Cirrus Video Controller](../QemuVideoDxe/ReadMe.md) |

### Libraries

| Libraries | Link to Documentation |
| --- | --- |
| **MsPlatformDevicesLib** | [MsPlatformDevicesLib](../Library/MsPlatformDevicesLibQemuQ35/ReadMe.md) |
| **PlatformDebugLibIoPort** | [PlatformDebugLibIoPort](../Library/PlatformDebugLibIoPort/ReadMe.md) |
| **PlatformThemeLib** | [PlatformThemeLib](../Library/PlatformThemeLib/ReadMe.md) |
