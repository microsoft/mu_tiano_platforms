# PXE Boot

## Overview

QemuQ35Pkg supports PXE (Preboot Execution Environment) booting using the firmware's
built-in UEFI network stack and QEMU's user-mode networking. This allows the virtual
machine to boot an EFI application served over TFTP without any external DHCP or TFTP
infrastructure — everything runs self-contained within the QEMU process.

## Table of Contents

- [Overview](#overview)
- [Firmware Network Stack](#firmware-network-stack)
- [NIC Selection](#nic-selection)
- [Usage](#usage)
- [How It Works](#how-it-works)
- [Boot Order](#boot-order)
- [QEMU Network Environment](#qemu-network-environment)
- [Building QEMU with Slirp](#building-qemu-with-slirp)
- [Limitations](#limitations)
- [Troubleshooting](#troubleshooting)

## Firmware Network Stack

QemuQ35Pkg includes a full IPv4 and IPv6 network stack with PXE support. The following
drivers are included in the firmware image:

| Driver               | Purpose                          |
|----------------------|----------------------------------|
| `VirtioNetDxe`       | VirtIO network device driver     |
| `SnpDxe`             | Simple Network Protocol          |
| `MnpDxe`             | Managed Network Protocol         |
| `ArpDxe`             | Address Resolution Protocol      |
| `Dhcp4Dxe`           | DHCPv4 client                    |
| `Ip4Dxe`             | IPv4 protocol                    |
| `Udp4Dxe`            | UDPv4 protocol                   |
| `Mtftp4Dxe`          | TFTP client (used by PXE)        |
| `TcpDxe`             | TCP protocol                     |
| `UefiPxeBcDxe`       | UEFI PXE Base Code               |
| `DnsDxe`             | DNS resolver                     |
| `HttpDxe`            | HTTP protocol                    |
| `HttpBootDxe`        | HTTP Boot support                |
| `TlsDxe`             | TLS support                      |
| `Dhcp6Dxe`           | DHCPv6 client                    |
| `Ip6Dxe`             | IPv6 protocol                    |
| `Udp6Dxe`            | UDPv6 protocol                   |
| `Mtftp6Dxe`          | TFTPv6 client                    |

Both IPv4 and IPv6 PXE are enabled via PCDs:

- `gEfiNetworkPkgTokenSpaceGuid.PcdIPv4PXESupport = 0x01`
- `gEfiNetworkPkgTokenSpaceGuid.PcdIPv6PXESupport = 0x01`

## NIC Selection

QemuRunner selects the QEMU NIC model based on the boot scenario:

| Scenario                             | NIC Model         | Reason                                         |
|--------------------------------------|-------------------|-------------------------------------------------|
| PXE boot (`PXE_BOOT_FILE` set)       | `virtio-net-pci`  | Firmware has `VirtioNetDxe` built in             |
| OS boot (no front page)              | `e1000e`          | Better guest OS driver coverage (e.g. Windows)   |
| Front page / UEFI shell              | `virtio-net-pci`  | Firmware has `VirtioNetDxe` built in             |

> **Note:** The firmware does not include an e1000/e1000e driver by default. An optional
> E1000 binary driver can be enabled with `BLD_*_E1000_ENABLE=1`, but PXE boot at the
> firmware level requires `virtio-net-pci` since `VirtioNetDxe` is always present.

## Usage

PXE boot requires two flags: `ENABLE_NETWORK=TRUE` and `PXE_BOOT_FILE=<path>`.

```bash
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=GCC5 \
    ENABLE_NETWORK=TRUE \
    PXE_BOOT_FILE=/path/to/tftp/root/bootx64.efi \
    --FlashOnly
```

The `PXE_BOOT_FILE` value is a path to the EFI boot file on the host filesystem:

- The **parent directory** becomes the TFTP root served by QEMU.
- The **filename** is advertised to the guest as the DHCP boot file.
- Relative paths are resolved to absolute paths automatically.

For example, given `PXE_BOOT_FILE=../tftp/bootx64.efi`:

- TFTP root: the absolute path to `../tftp/`
- Boot file: `bootx64.efi`

### Combined with Acceleration

PXE boot works with any acceleration mode:

```bash
# PXE boot with KVM acceleration
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=GCC5 \
    QEMU_ACCEL=kvm CPU_MODEL=host \
    ENABLE_NETWORK=TRUE \
    PXE_BOOT_FILE=/path/to/bootx64.efi \
    --FlashOnly
```

## How It Works

When `PXE_BOOT_FILE` is set, `QemuRunner` makes the following changes to the QEMU
command line:

1. **Skips VirtualDrive** — The virtual disk containing `startup.nsh` is not mounted,
   preventing the UEFI shell from intercepting the boot.

2. **Configures TFTP/DHCP** — The `-netdev user` argument is extended with
   `tftp=<root>,bootfile=<name>` to enable QEMU's built-in DHCP and TFTP servers.

3. **Selects virtio-net-pci** — The NIC is set to `virtio-net-pci` so the firmware's
   built-in `VirtioNetDxe` driver can bind to it.

4. **Enables alternate boot order** — The SMBIOS type=3 `version` field is set to
   `Vol-`, which triggers the firmware's alternate boot sequence
   (USB → PXE IPv4 → PXE IPv6 → HDD). Since no USB bootable media is present, the
   firmware falls through to PXE IPv4 automatically.

## Boot Order

The firmware determines boot priority via the SMBIOS type=3 `version` field, which
is interpreted by `FrontPageButtons` as a virtual button press:

| Version String | Virtual Button | Boot Behavior                                    |
|----------------|----------------|--------------------------------------------------|
| `Vol+`         | Volume Up      | Boot to Front Page (settings UI)                 |
| `Vol-`         | Volume Down    | Alternate boot: USB → PXE4 → PXE6 → HDD         |
| *(none)*       | No buttons     | Normal boot order: HDD → USB → PXE (lowest)     |

When `PXE_BOOT_FILE` is set and no explicit `BOOT_TO_FRONT_PAGE` or `ALT_BOOT_ENABLE`
flag is provided, QemuRunner automatically sets `Vol-` to ensure PXE is reached early
in the boot sequence.

The alternate boot sequence is defined in `MsBootPolicyLib`:

```
BootSequenceUPH = { USB, PXE IPv4, PXE IPv6, HDD }
```

This is configured in
`Common/MU_OEM_SAMPLE/OemPkg/Library/MsBootPolicyLib/MsBootPolicyLib.c`.

## QEMU Network Environment

QEMU's user-mode (slirp) networking provides a self-contained network environment:

| Service       | Address       | Description                                |
|---------------|---------------|--------------------------------------------|
| Gateway/DHCP  | `10.0.2.2`    | DHCP server, assigns `10.0.2.15` to guest  |
| DNS           | `10.0.2.3`    | DNS forwarder                              |
| TFTP          | `10.0.2.2`    | Serves files from the `PXE_BOOT_FILE` parent directory |
| Guest IP      | `10.0.2.15`   | IP assigned to the guest via DHCP          |

The firmware boot flow is:

1. `VirtioNetDxe` binds to the `virtio-net-pci` device.
2. `UefiPxeBcDxe` starts a PXE boot attempt on the network interface.
3. `Dhcp4Dxe` sends a DHCP Discover and receives an offer from QEMU (`10.0.2.2`).
4. The DHCP response includes the boot file name (e.g., `bootx64.efi`).
5. `Mtftp4Dxe` downloads the boot file from the TFTP server at `10.0.2.2`.
6. The firmware executes the downloaded EFI application.

## Building QEMU with Slirp

QEMU's user-mode networking requires `libslirp`. If you build QEMU from source,
ensure slirp support is included:

```bash
# Install the slirp development library
sudo apt-get install libslirp-dev

# Configure QEMU with slirp enabled
./configure --target-list=x86_64-softmmu --enable-slirp
make -j$(nproc)
```

Without `--enable-slirp`, QEMU will fail with:

```
qemu-system-x86_64: network backend 'user' is not compiled into this binary
```

## Limitations

- **User-mode networking only** — QEMU's built-in DHCP/TFTP is only available with
  the `user` (slirp) network backend. TAP or bridge networking requires external
  DHCP/TFTP servers.

- **No inbound connections** — The guest cannot receive unsolicited inbound connections
  from the host with user-mode networking. Use `-netdev user,...,hostfwd=` for port
  forwarding if needed.

- **Single NIC** — QemuRunner currently configures a single network interface. The NIC
  model is selected automatically based on the boot scenario (see [NIC Selection](#nic-selection)).

- **IPv4 PXE only with QEMU DHCP** — QEMU's built-in DHCP server only supports IPv4.
  IPv6 PXE requires an external DHCPv6/TFTP infrastructure.

## Troubleshooting

### PXE boot goes to UEFI Shell instead of booting

The VirtualDrive containing `startup.nsh` may still be mounted. Ensure `PXE_BOOT_FILE`
is set — QemuRunner automatically skips the VirtualDrive when this flag is present.

### "network backend 'user' is not compiled into this binary"

QEMU was built without slirp support. Rebuild QEMU with `--enable-slirp` and ensure
`libslirp-dev` is installed. See [Building QEMU with Slirp](#building-qemu-with-slirp).

### No network adapter found in firmware

Ensure `ENABLE_NETWORK=TRUE` is set. When `PXE_BOOT_FILE` is configured, QemuRunner
uses `virtio-net-pci` which requires the firmware's built-in `VirtioNetDxe` driver.
If using an e1000/e1000e NIC, the firmware won't have a driver unless
`BLD_*_E1000_ENABLE=1` is also set.

### PXE boot attempted but DHCP times out

- Verify the TFTP root directory exists and contains the boot file.
- Check that the path in `PXE_BOOT_FILE` is correct (relative paths are resolved
  automatically).
- Ensure QEMU's slirp networking is functional by checking the QEMU log for
  network-related errors.

### PXE is not selected automatically (boots to HDD or shell)

QemuRunner sets `ALT_BOOT_ENABLE` (via SMBIOS `Vol-`) when `PXE_BOOT_FILE` is
configured and no other boot selection is explicitly set. If you also set
`BOOT_TO_FRONT_PAGE=TRUE`, the front page takes precedence. Remove conflicting
boot flags to allow PXE auto-selection.
