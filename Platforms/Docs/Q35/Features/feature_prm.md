# Platform Runtime Mechanism (PRM)

## Overview

Platform Runtime Mechanism (PRM) introduces the capability of moving certain classes of SMM code out of SMM and into
a code module that executes within OS context. Generally, SMM code that does not depend upon SMM execution privileges
is a candidate for conversion to PRM. The PRM conversion process involves porting code from SMM modules into code
modules that execute at CPL0 and are directly invoked by host OS kernel components. These code modules are called
**PRM Modules**. Functions within a PRM Module exposed to the OS for runtime execution are called **PRM Handlers**.

End-to-end PRM support on a system requires firmware and OS support. The firmware must provide an initial set of PRM
modules for the OS to use along with an ACPI tabled called PRMT that describes those modules. The OS uses this
information to invoke PRM functionality when requested by a kernel component. PRM has two high-level invocation
paths either directly from an OS driver (direct call) or by interacting with an ACPI OpRegion (ACPI call).

## PRM Goal in QemuQ35Pkg

The goals of the PRM feature within `QemuQ35Pkg` are:

1. To serve as an open source example of how to integrate PRM into a platform firmware.
2. To serve as a test vehicle for generic PRM infrastructure and new PRM handlers testable in a virtual system.
3. To provide an easily accessible virtual environment in open source that lends to PRM feature experimentation.

For more information about the PRM feature and to access the platform agnostic PRM code/documentation visit
[edk2-staging/PlatformRuntimeMechanism](https://github.com/tianocore/edk2-staging/tree/PlatformRuntimeMechanism).

## PRM Platform Agnostic Feature Code

The official edk2 support for PRM is being developed in
[edk2-staging/PlatformRuntimeMechanism](https://github.com/tianocore/edk2-staging/tree/PlatformRuntimeMechanism).

All of the content in that branch is agnostic to any particular platform and should be considered the single source
for PRM firmware infrastructure in an edk2 based firmware.

## PRM Modules Overview

PRM is adopted in a particular platform by including platform-agnostic components from `PrmPkg` and then supplementing
that with the PRM Modules that perform some platform-specific work.

The following are key platform agnostic modules.

- `PrmLoaderDxe` - Discovers PRM Modules loaded into memory by the platform (e.g. an FV with PRM Modules is installed)
  and places those modules and the PRM Handlers within those modules into the PRMT ACPI table so the PRM configuration
  for the platform is described to the operating system.

- `PrmConfigDxe` - Configures PRM Module settings during the boot services environment.

  Some modules need special configuration and others do not. For example, if a module needs MMIO ranges to be converted
  it would describe those MMIO ranges during boot services so they are converted in the virtual memory address change
  event. Another example would be a module that allocates a static data buffer and then populates it with some data
  like that from a Setup menu item or a RAW section in a FV so it's accessible to a PRM Handler later.

  Often each PRM Module links a configuration library against this module to perform the configuration work needed for
  the module. The PRM Module could also choose to create a dedicated DXE configuration driver if that's preferred.

  For example, here's the configuration libraries currently linked against `PrmConfigDxe` in `QemuQ35Pkg`:

  ```inf
  PrmPkg/PrmConfigDxe/PrmConfigDxe.inf {
    <LibraryClasses>
      NULL|PrmPkg/Samples/PrmSampleAcpiParameterBufferModule/Library/DxeAcpiParameterBufferModuleConfigLib/DxeAcpiParameterBufferModuleConfigLib.inf
      NULL|PrmPkg/Samples/PrmSampleContextBufferModule/Library/DxeContextBufferModuleConfigLib/DxeContextBufferModuleConfigLib.inf
      NULL|PrmPkg/Samples/PrmSampleHardwareAccessModule/Library/DxeHardwareAccessModuleConfigLib/DxeHardwareAccessModuleConfigLib.inf
  }
  ```

- `PrmSsdtInstallDxe` - Installs the PRM SSDT. The SSDT in `PrmPkg` is a reference SSDT and a platform owner should
  inspect the SSDT to determine whether any changes are required. If the platform will not trigger PRM Handlers from
  ACPI code at all (only use direct call), this driver can be excluded from the platform firmware.

- `PrmInfo` - An optional UEFI application that reports information about the PRM configuration currently loaded in
  the system. The application can be used to confirm PRM Modules are discovered correctly and to exercise PRM Handlers
  in a lightweight manner (some activities like updating parameter buffers cannot be performed).

In order to incorporate some PRM Modules into the boot flow, the sample PRM Modules provided by `PrmPkg` are loaded
by `QemuQ35Pkg`.

## PRM Libraries Overview

Some aspects of the generic PRM Modules in `PrmPkg` are customizable with libraries. It is not expected a platform
needs to provide custom libraries but it is possible if needed. Those libraries are briefly noted below.

- `PrmContextBufferLib` - Provides a general abstraction for PRM context buffer management.

- `PrmModuleDiscoveryLib` - Provides functionality to discover PRM modules loaded in the system boot.

- `PrmPeCoffLib` - Provides functionality to support additional PE/COFF functionality needed to use Platform Runtime
  Mechanism (PRM) modules.
