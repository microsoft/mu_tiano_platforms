# PXE Boot

## Overview

QemuQ35Pkg integrates PXE boot support through the `QemuRunner` plugin and the platform's network stack configuration.
The plugin handles QEMU command-line construction for TFTP/DHCP, NIC selection, VirtualDrive suppression, and automatic
boot order manipulation so that a single `stuart_build --FlashOnly` invocation can PXE boot an EFI application from a
host-local file.

QEMU's user-mode (slirp) networking provides the DHCP and TFTP services. See the
[QEMU networking documentation](https://www.qemu.org/docs/master/system/devices/net.html) for details on slirp
behavior and limitations.

## Usage

PXE boot requires `ENABLE_NETWORK` and `PXE_BOOT_FILE`:

```bash
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=CLANGPDB \
    ENABLE_NETWORK=TRUE \
    PXE_BOOT_FILE=/path/to/tftp/root/bootx64.efi \
    --FlashOnly
```

`PXE_BOOT_FILE` is resolved to an absolute path. The parent directory becomes the TFTP root and the filename becomes
the DHCP boot file advertised to the guest. For example, `PXE_BOOT_FILE=../tftp/bootx64.efi` yields
`tftp=<abs path to ../tftp/>` and `bootfile=bootx64.efi` on the QEMU `-netdev user` argument.

## QemuRunner Behavior

When `PXE_BOOT_FILE` is set, QemuRunner.py changes the QEMU command line in four ways:

1. The VirtualDrive (containing `startup.nsh`) is not mounted, preventing the UEFI shell from intercepting the boot.

2. The `-netdev user` argument is extended with `tftp=<root>,bootfile=<name>`.

3. The NIC is forced to `virtio-net-pci` because the firmware includes `QemuPkg/VirtioNetDxe/VirtioNet.inf`. Without
   `PXE_BOOT_FILE`, QemuRunner selects the NIC based on context:

   | Scenario                  | NIC             |
   |---------------------------|-----------------|
   | `PXE_BOOT_FILE` set       | `virtio-net-pci`|
   | OS boot (no front page)   | `e1000e`        |
   | Front page / UEFI shell   | `virtio-net-pci`|

4. If no explicit `BOOT_TO_FRONT_PAGE` or `ALT_BOOT_ENABLE` flag is provided, the SMBIOS type=3 `version` field is set
   to `Vol-`. This triggers the alternate boot sequence defined in
   `Common/MU_OEM_SAMPLE/OemPkg/Library/MsBootPolicyLib/MsBootPolicyLib.c`:

   ```c
   static BOOT_SEQUENCE  BootSequenceUPH[] = {
     MsBootUSB,
     MsBootPXE4,
     MsBootPXE6,
     MsBootHDD,
     MsBootDone
   };
   ```

   Since no USB bootable media is present, the firmware falls through to PXE IPv4.

## Platform Build Flags

| Flag                      | Scope              | Effect                                                                                                                   |
|---------------------------|--------------------|--------------------------------------------------------------------------------------------------------------------------|
| `ENABLE_NETWORK=TRUE`     | QemuRunner env var | Adds `-netdev user` and a NIC device to the QEMU command line. Without this, networking is disabled (`-net none`).       |
| `PXE_BOOT_FILE=<path>`    | QemuRunner env var | Enables TFTP/DHCP args, selects `virtio-net-pci`, suppresses VirtualDrive, auto-sets alternate boot.                     |
