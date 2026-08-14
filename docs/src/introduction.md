# Introduction

**Welcome to the Mu Tiano Platforms book.** This book documents the two virtual platforms
maintained in the [`mu_tiano_platforms`](https://github.com/microsoft/mu_tiano_platforms)
repository and the workflows used to build, debug, and validate them.

The primary purpose of `mu_tiano_platforms` is to provide readily available, free, and
feature-rich reference platforms built with [Project Mu](https://microsoft.github.io/mu)
that target the open-source [QEMU](https://www.qemu.org/) processor emulator. These
platforms serve as an example for feature enablement and validation — demonstrating how
a single firmware codebase can be shared across multiple products and architectures,
promoting serviceable, maintainable, up-to-date, and secure firmware. A goal of this
repository is to reduce the overhead of testing and evaluating common functionality
before deployment to physical hardware.

`QemuQ35Pkg` in this repository was originally derived from
[`OvmfPkg`](https://github.com/tianocore/edk2/tree/HEAD/OvmfPkg) in TianoCore. The
package is considered stable, so regular syncing is not performed with the upstream;
select changes are cherry-picked based on functional or security importance.

**Platforms**:

- [QEMU Q35](./platforms/q35.md) - an IA32/X64 platform based on the Intel Q35 chipset.
- [QEMU Arm Virt](./platforms/arm_virt.md) - an AArch64 platform based on the QEMU ARM
  Virtual Machine.

## What is in this book

This book is organized into the following sections:

1. **[Building and Debugging](building/building.md)** - Setting up your environment,
   compiling the firmware, running it in QEMU, and connecting a debugger to inspect the
   boot flow.
2. **[Common Features](features/common/feature_dfci.md)** - Project Mu features enabled
   across both QEMU platforms: DFCI, Front Page, memory protection, telemetry (WHEA),
   TPM, the platform testing framework, and the shared `QemuRunner` plugin.
3. **[Q35 Features](features/q35/feature_codeql.md)** - Q35-specific features including
   CodeQL static analysis, the Color Bar device-state indicator, the configuration
   (setup data) framework, Platform Runtime Mechanism (PRM), and TPM Replay.
4. **[Trusted Platform Module (TPM)](tpm/tpm_q35.md)** - Deep dives on the TPM 2.0 stack
   for each platform, including the FF-A based dual-CRB architecture on Arm Virt and the
   direct CRB/TIS path on Q35, plus guidance for running `TpmShellApp` on both.
5. **[Templates](template/branch_template.md)** - Reusable templates for feature-branch
   documentation.

## Related Documentation

- [Project Mu](https://microsoft.github.io/mu) - the main Project Mu documentation site.
- [Project Mu on GitHub](https://github.com/microsoft?q=mu_&type=all) - hosts all Project
  Mu repositories, including this one, `mu_basecore`, `mu_plus`, `mu_oem_sample`, and
  the feature repos consumed as submodules here.
- [QEMU](https://www.qemu.org/) - the open-source processor emulator these platforms
  target.
- [How to Build in EDK II with Stuart][stuart-wiki] - background on the Stuart build
  system used throughout this repository.
- [Releases](https://github.com/microsoft/mu_tiano_platforms/releases),
  [Discussions](https://github.com/microsoft/mu_tiano_platforms/discussions),
  [Issues](https://github.com/microsoft/mu_tiano_platforms/issues), and the
  [security policy](https://github.com/microsoft/mu_tiano_platforms/security/policy) for
  this repository.

[stuart-wiki]: https://github.com/tianocore/tianocore.github.io/wiki/How-to-Build-With-Stuart
