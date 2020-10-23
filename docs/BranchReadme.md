# BranchReadme PRM

Platform Runtime Mechanism (PRM) introduces the capability of moving certain classes of SMM code out of SMM and into
a code module that executes within OS context. Generally, SMM code that does not depend upon SMM execution privileges
is a candidate for conversion to PRM. The PRM conversion process involves porting code from SMM modules into code
modules that execute at CPL0 and are directly invoked by host OS kernel components. These code modules are called
**PRM Modules**. Functions within a PRM Module exposed to the OS for runtime execution are called **PRM Handlers**.

End-to-end PRM support on a system requires firmware and OS support. The firmware must provide an initial set of PRM
modules for the OS to use along with an ACPI tabled called PRMT that describes those modules. The OS uses this
information to invoke PRM functionality when requested by a kernel component. PRM has two high-level invocation
paths either directly from an OS driver (direct call) or by interacting with an ACPI OpRegion (ACPI call).

The official edk2 support for PRM is being developed in
[edk2-staging/PlatformRuntimeMechanism](https://github.com/tianocore/edk2-staging/tree/PlatformRuntimeMechanism).

All of the content in that branch is agnostic to any particular platform and should be considered the single source
for PRM firmware infrastructure in an edk2 based firmware. As such, the goal of the PRM feature within `QemuQ35Pkg`
is to:

1. Serve as an open source example of how to integrate PRM into a platform firmware.
2. Serve as a test vehicle for generic PRM infrastructure and new PRM handlers testable in a virtual system.
3. Provide an easily accessible virtual environment in open source that lends to PRM feature experimentation.

For more information about the PRM feature and to access the platform agnostic PRM code/documentation visit
[edk2-staging/PlatformRuntimeMechanism](https://github.com/tianocore/edk2-staging/tree/PlatformRuntimeMechanism).

Branched from: 202008

Branched on: 10/23/2020

Author: michael.kubacki@microsoft.com

## What's unique about this branch

Adds support for PRM functionality in QemuQ35Pkg.

## Roadmap

This branch achieves the goals envisioned for the branch. That is to execute the PRM infrastructure on QEMU
and successfully load the open source PRM sample modules available in
[PrmPkg/Samples](https://github.com/tianocore/edk2-staging/tree/PlatformRuntimeMechanism/PrmPkg/Samples).

## Expected lifetime of your branch

Very short - for the lifetime of the PR review to merge into release/202008.
