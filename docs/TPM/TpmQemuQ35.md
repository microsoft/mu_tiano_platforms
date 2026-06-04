# TPM on QEMU Q35

This document describes the TPM 2.0 architecture for the QEMU Q35 platform. Q35 uses a
direct CRB/FIFO path between firmware and the TPM device.

## Table of Contents

- [Requirements](#requirements)
- [Build Configuration](#build-configuration)
- [Platform Memory Layout](#platform-memory-layout)
- [Architecture Overview](#architecture-overview)
- [TPM Device Library Stack](#tpm-device-library-stack)
- [Hash Library Architecture](#hash-library-architecture)
- [Physical Presence Interface](#physical-presence-interface)
- [swtpm Setup](#swtpm-setup)
- [Communication Flow](#communication-flow)
- [PCDs Reference](#pcds-reference)

## Requirements

| Requirement | Notes |
| ------------- | ------- |
| **Host OS** | Linux (native) or **WSL** on Windows. Native Windows is not supported. |
| **swtpm** | TPM 2.0 emulator. Install via your distro's package manager (e.g. `apt install swtpm swtpm-tools`). |
| **QEMU** | Built with `tpm-tis` device support (standard upstream QEMU includes this). |
| **Build host** | Same Linux/WSL environment used to run `stuart_build` and launch QEMU. |

See [swtpm Setup](#swtpm-setup) for the full setup commands.

## Build Configuration

The TPM is disabled by default. To enable it, set `BLD_*_TPM_ENABLE=TRUE` and `TPM_DEV=TRUE`
on the command line or in a BuildConfig.conf file placed at the root level of the repo:

```bash
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py --FlashRom \
  BLD_*_TPM_ENABLE=TRUE \
  TPM_DEV=TRUE
```

The following defines control TPM behavior in `QemuQ35Pkg.dsc`:

| Define | Default | Purpose |
| -------- | --------- | --------- |
| `TPM_ENABLE` | `FALSE` | Master switch. Guards all TPM drivers, libraries, and PCDs. |
| `TPM_CONFIG_ENABLE` | `FALSE` | Enables `Tcg2ConfigDxe` HII configuration UI. |
| `TPM_REPLAY_ENABLED` | `FALSE` | Enables TPM Replay overrides (uses `TpmTestingPkg` variants of Tcg2Dxe and DxeTpm2MeasureBootLib). |

## Platform Memory Layout

Q35 uses the standard x86 TPM memory-mapped I/O region:

| Region | Address | Size | Interface |
| -------- | --------- | ------ | ----------- |
| TPM TIS/CRB | `0xFED40000` | 0x5000 (20 KiB) | CRB or TIS (auto-detected) |

The firmware communicates directly with the TPM device via MMIO, and QEMU forwards the
I/O to swtpm over a Unix socket.

The base address comes from the SecurityPkg package declaration default
(`PcdTpmBaseAddress = 0xFED40000`). The Q35 DSC does not override it explicitly. The
`Tcg2ConfigPei` driver detects the TPM at this address during PEI.

## Architecture Overview

```text
┌──────────────────────────────────────────────────────────────────────────────────┐
│ UEFI Firmware (x86_64)                                                           │
│                                                                                  │
│  ┌─── PEI Phase ──────────────────────────────────────────────────────────────┐  │
│  │                                                                            │  │
│  │  Tcg2ConfigPei                                                             │  │
│  │    │ 1. Detect TPM 1.2 vs 2.0 at 0xFED40000                                │  │
│  │    │ 2. Set PcdTpmInstanceGuid                                             │  │
│  │    ▼                                                                       │  │
│  │  HashLibBaseCryptoRouterPei + HashInstanceLib*                             │  │
│  │    │ 1. Constructors register each enabled hash algorithm                  │  │
│  │    │ 2. Filtered by PcdTpm2HashMask → PcdTcg2HashAlgorithmBitmap           │  │
│  │    ▼                                                                       │  │
│  │  Tcg2Pei                                                                   │  │
│  │    │ 1. Tpm2RequestUseTpm()                                                │  │
│  │    │ 2. Tpm2Startup(TPM_SU_CLEAR)                                          │  │
│  │    │ 3. SyncPcrAllocationsAndPcrMask()                                     │  │
│  │    │ 4. Tpm2SelfTest()                                                     │  │
│  │    │ 5. Measure firmware volumes (CRTM) into PCR[0-7]                      │  │
│  │    │ 6. Install TpmInitializedPpi                                          │  │
│  └────┼───────────────────────────────────────────────────────────────────────┘  │
│       ▼                                                                          │
│  ┌─── DXE Phase ──────────────────────────────────────────────────────────────┐  │
│  │                                                                            │  │
│  │  HashLibBaseCryptoRouterDxe + HashInstanceLib*                             │  │
│  │    │ 1. Constructors register each enabled hash algorithm                  │  │
│  │    │ 2. Filtered by PcdTpm2HashMask → PcdTcg2HashAlgorithmBitmap           │  │
│  │    ▼                                                                       │  │
│  │  Tcg2Dxe                                                                   │  │
│  │    │ 1. Verify PcdTpmInstanceGuid is TPM 2.0                               │  │
│  │    │ 2. Verify no TpmErrorHob is present                                   │  │
│  │    │ 3. Tpm2RequestUseTpm()                                                │  │
│  │    │ 4. Query TPM capabilities                                             │  │
│  │    │     ├── Manufacturer                                                  │  │
│  │    │     ├── Firmware version                                              │  │
│  │    │     └── Max cmd/resp size                                             │  │
│  │    │ 5. Get supported/active PCR banks filtered by → HashAlgorithmBitmap   │  │
│  │    │ 6. Decide SupportedEventLogs (TCG_1_2 only if SHA1 active)            │  │
│  │    │ 7. SetupEventLog                                                      │  │
│  │    │     ├── Allocate log area(s)                                          │  │
│  │    │     └── Acquire and log pre-DXE HOB(s)                                │  │
│  │    │ 8. Register events                                                    │  │
│  │    │     ├── ReadyToBoot                                                   │  │
│  │    │     ├── ExitBootServices                                              │  │
│  │    │     └── ExitBootServices Failed                                       │  │
│  │    │ 9. Register protocol notifies                                         │  │
│  │    │     ├── VariableWriteArch (SecureBoot)                                │  │
│  │    │     └── ResetNotification (TPM shutdown)                              │  │
│  │    │ 10. Install Tcg2Protocol                                              │  │
│  └────┼───────────────────────────────────────────────────────────────────────┘  │
│       ▼                                                                          │
│  ┌─── BDS Phase ──────────────────────────────────────────────────────────────┐  │
│  │                                                                            │  │
│  │  DeviceBootManagerAfterConsole                                             │  │
│  │    │ 1. Tcg2PhysicalPresenceLibProcessRequest (NULL)                       │  │
│  │    │ 2. Process any pending PP request before shell launch                 │  │
│  │    │ 3. Create TCG2_PHYSICAL_PRESENCE_VARIABLE if it doesn't exist         │  │
│  └────┼───────────────────────────────────────────────────────────────────────┘  │
│       ▼                                                                          │
│  ┌─── UEFI Shell ─────────────────────────────────────────────────────────────┐  │
│  │                                                                            │  │
│  │  UEFI Shell / OS / TpmShellApp                                             │  │
│  │    │ 1. gBS->LocateProtocol(&gEfiTcg2ProtocolGuid)                         │  │
│  │    │ 2. Tcg2Protocol->GetCapability / SetActivePcrBanks / etc.             │  │
│  └────┼───────────────────────────────────────────────────────────────────────┘  │
│       ▼                                                                          │
│  Tpm2DeviceLibDTpm ─ direct MMIO reads/writes to 0xFED40000                      │
│       │                                                                          │
├───────┼──────────────────────────────────────────────────────────────────────────┤
│       ▼                                                                          │
│  QEMU TPM TIS device (-device tpm-tis,tpmdev=tpm0)                               │
│       │                                                                          │
│       ▼                                                                          │
│  Unix socket ─ swtpm process (--tpm2)                                            │
└──────────────────────────────────────────────────────────────────────────────────┘
```

## TPM Device Library Stack

Q35 uses two different patterns for accessing the TPM, depending on the firmware phase:

### PEI Phase: Direct MMIO (`Tpm2DeviceLibDTpm`)

```text
Tcg2Pei
  │ Tpm2SubmitCommand()
  ▼
Tpm2DeviceLibDTpm
  │ Auto-detects CRB vs TIS vs FIFO via InterfaceId register
  │ DTpm2SubmitCommand() dispatches:
  │   CRB  → PtpCrbTpmCommand()
  │   FIFO → Tpm2TisTpmCommand()
  │   TIS  → Tpm2TisTpmCommand()
  ▼
Direct MMIO to 0xFED40000
```

`Tpm2DeviceLibDTpm` reads the `InterfaceId` register at `PcdTpmBaseAddress + 0x30` to
determine the interface type. QEMU's `tpm-tis` device presents a TIS/FIFO interface.

### DXE Phase: Router Pattern (`Tpm2DeviceLibRouter`)

```text
Tcg2Dxe
  │ Tpm2SubmitCommand()
  ▼
Tpm2DeviceLibRouterDxe
  │ Delegates to registered TPM2_DEVICE_INTERFACE
  ▼
Tpm2InstanceLibDTpm (constructor registers with router)
  │ DTpm2SubmitCommand() — same dispatch as PEI
  ▼
Direct MMIO to 0xFED40000
```

The router pattern exists to support future scenarios where multiple TPM device types
could coexist. The router accepts only the instance whose `ProviderGuid` matches
`PcdTpmInstanceGuid`.

### Other DXE Drivers: TCG2 Protocol (`Tpm2DeviceLibTcg2`)

Most DXE drivers that need TPM access use `Tpm2DeviceLibTcg2`, which goes through the
TCG2 Protocol rather than direct MMIO. Only Tcg2Dxe itself uses `Tpm2DeviceLibRouter`
with direct MMIO.

### CRB Register Layout

If the TPM presents a CRB interface (as opposed to TIS/FIFO), the register layout is
defined by the TCG PC Client Platform TPM Profile (PTP) specification. See:

- [TCG PC Client Platform TPM Profile (PTP) Specification][ptp-spec] —
  *Section 6 "Command Response Buffer Interface"* describes `LocalityState`,
  `LocalityControl`, `InterfaceId`, `CrbControlRequest`, `CrbControlStart`,
  `CrbControlCommand*`/`CrbControlResponse*`, and the shared `CrbDataBuffer`.

The key register for this platform is `InterfaceId` at `PcdTpmBaseAddress + 0x30`, which
`Tpm2DeviceLibDTpm` reads to decide between CRB and TIS/FIFO dispatch paths.

### TIS Register Layout

QEMU's `tpm-tis` device presents a TIS (TPM Interface Specification) / FIFO interface.
The register layout is defined by:

- [TCG PC Client Specific TPM Interface Specification (TIS)][tis-spec] —
  defines `Access`, `IntEnable`/`IntVector`, `STS` (with `commandReady`, `tpmGo`,
  `dataAvail`, `burstCount`), `DataFifo`, and the `Vid`/`Did`/`Rid` identification
  registers.

The TIS flow uses `BurstCount` from the STS register to pace reads and writes through
the `DataFifo` register, one burst at a time.

[ptp-spec]: https://trustedcomputinggroup.org/resource/pc-client-platform-tpm-profile-ptp-specification/
[tis-spec]: https://trustedcomputinggroup.org/resource/pc-client-work-group-pc-client-specific-tpm-interface-specification-tis/

## Hash Library Architecture

Tcg2Dxe uses `HashLibBaseCryptoRouterDxe` while Tcg2Pei uses `HashLibBaseCryptoRouterPei`
with all hash instance libraries included.

### Registration Flow

1. `HashLibBaseCryptoRouterConstructor` resets `PcdTcg2HashAlgorithmBitmap` to 0.
2. Each `HashInstanceLib` constructor calls `RegisterHashInterfaceLib()`.
3. `RegisterHashInterfaceLib()` checks the algorithm against `PcdTpm2HashMask` (`0x02` =
   SHA256 only). Algorithms not in the mask return `EFI_UNSUPPORTED`.

### Hash Algorithm Bitmask Values

The bit positions used in `PcdTpm2HashMask`, `PcdTcg2HashAlgorithmBitmap`, and the
`EFI_TCG2_BOOT_SERVICE_CAPABILITY.HashAlgorithmBitmap` field are defined by the EFI
TCG2 protocol and the TCG algorithm registry:

- [UEFI TCG2 Protocol Specification][tcg2-proto] — see `EFI_TCG2_BOOT_HASH_ALG_*`
  (`SHA1` = BIT0, `SHA256` = BIT1, `SHA384` = BIT2, `SHA512` = BIT3, `SM3_256` = BIT4).
- [TCG Algorithm Registry][tcg-algreg] — canonical list of TPM hash algorithm IDs.

For this platform, `PcdTpm2HashMask = 0x02` enables SHA256 only.

[tcg2-proto]: https://trustedcomputinggroup.org/resource/tcg-efi-protocol-specification/
[tcg-algreg]: https://trustedcomputinggroup.org/resource/tcg-algorithm-registry/

### Filtering Chain

```text
PcdTpm2HashMask (0x02)
        │
        ▼
RegisterHashInterfaceLib() ── gates which HashInstanceLibs register
        │
        ▼
PcdTcg2HashAlgorithmBitmap ── result of all successful registrations
        │
        ▼
Tcg2Dxe intersects with TPM-reported capabilities
        │
        ▼
Final ActivePcrBanks / HashAlgorithmBitmap in EFI_TCG2_BOOT_SERVICE_CAPABILITY
```

## Physical Presence Interface

### Library Selection

| `TPM_ENABLE` | Library | Behavior |
| -------------- | --------- | ---------- |
| `FALSE` | `Tcg2PhysicalPresenceLibNull` | All functions stubbed |
| `TRUE` | `DxeTcg2PhysicalPresenceMinimumLib` | Auto-confirms Clear; rejects all other operations |

The MinimumLib implementation:

- **Auto-confirms** TPM Clear operations without user prompting.
- **Rejects** SET_PCR_BANKS, LOG_ALL_DIGESTS, and other operations with
  `TCG_PP_RETURN_TPM_OPERATION_RESPONSE_FAILURE`.
- Does **not** create or use `TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE`.

### ProcessRequest in BDS

`Tcg2PhysicalPresenceLibProcessRequest()` is invoked from the platform's
`DeviceBootManagerLib` during `DeviceBootManagerAfterConsole()`, before the shell
launches. It:

1. Reads the `Tcg2PhysicalPresence` NV variable (creates it if missing).
2. Executes any pending PP request stored in the variable.
3. Stores the result back for `ReturnOperationResponseToOsFunction()` to report.

## swtpm Setup

### Installation

swtpm requires Unix sockets, so it must run in a Linux environment. On Windows,
use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) (Windows
Subsystem for Linux).

```bash
# Windows (from a WSL terminal)
wsl --install          # if WSL is not yet enabled
wsl                    # enter the WSL environment

# Ubuntu/Debian (native or WSL)
sudo apt install swtpm swtpm-tools

# Fedora
sudo dnf install swtpm swtpm-tools
```

### Manual Setup

Create the TPM state directory and start swtpm before launching QEMU:

```bash
mkdir -p /tmp/mytpm1
swtpm socket \
  --tpmstate dir=/tmp/mytpm1 \
  --ctrl type=unixio,path=/tmp/mytpm1/swtpm-sock \
  --tpm2 \
  --log level=20
```

### Automatic Setup (QemuRunner)

When `TPM_DEV=TRUE`, `QemuRunner.py` automatically starts swtpm in a background thread
before launching QEMU. The swtpm state directory is set to `BUILD_OUTPUT_BASE` and the
Unix socket is placed at `{BUILD_OUTPUT_BASE}/swtpm-sock`:

```python
# Platforms/QemuQ35Pkg/Plugins/QemuRunner/QemuRunner.py
@staticmethod
def RunThread(env):
    sw_tpm_enable = env.GetValue("TPM_DEV", "FALSE")
    if str(sw_tpm_enable).upper() == "FALSE":
        logging.critical("SWTPM Disabled")
        return

    tpm_dir = env.GetValue("BUILD_OUTPUT_BASE")
    tpm_sock = os.path.join(tpm_dir, "swtpm-sock")
    tpm_cmd = "swtpm"
    tpm_args = f"socket --tpmstate dir={tpm_dir} --ctrl type=unixio,path={tpm_sock} --tpm2 --log level=1"
    utility_functions.RunCmd(tpm_cmd, tpm_args)
```

The thread is launched before QEMU starts and joined after QEMU exits. The user is
prompted to press Ctrl+C to terminate swtpm at shutdown.

### QEMU Arguments

When `TPM_DEV=TRUE`, `QemuRunner.py` adds the following three arguments to the QEMU
command line for Q35 (with the socket path under `BUILD_OUTPUT_BASE`):

```text
-chardev socket,id=chrtpm,path={BUILD_OUTPUT_BASE}/swtpm-sock
-tpmdev emulator,id=tpm0,chardev=chrtpm
-device tpm-tis,tpmdev=tpm0
```

The `-device tpm-tis` argument is Q35-specific — it attaches a TIS-compatible TPM device
to the Q35 chipset at the standard address `0xFED40000`.

## Communication Flow

Complete path from a shell application to swtpm:

```text
TpmShellApp (UEFI Shell)
  │ gBS->LocateProtocol(&gEfiTcg2ProtocolGuid)
  │ Tcg2Protocol->SetActivePcrBanks(0x02)
  ▼
Tcg2Dxe (EFI_TCG2_PROTOCOL)
  │ Validates bank mask against HashAlgorithmBitmap
  │ Calls Tcg2PhysicalPresenceLibSubmitRequestToPreOSFunction()
  │   ├── MinimumLib: rejects SET_PCR_BANKS → returns EFI_UNSUPPORTED
  │   └── MinimumLib: NO_ACTION (already-active) → writes NV variable → EFI_SUCCESS
  ▼
Tpm2CommandLib (for direct TPM commands like GetCapability)
  │ Serializes TPM2 command structure into byte buffer
  │ Calls Tpm2SubmitCommand(cmdBuffer, cmdSize, rspBuffer, &rspSize)
  ▼
Tpm2DeviceLibRouter → Tpm2InstanceLibDTpm
  │ DTpm2SubmitCommand() — detects interface type (TIS on QEMU)
  ▼
Tpm2TisTpmCommand (Tpm2Tis.c)
  │ 1. TisPcPrepareCommand: set TIS_PC_STS_COMMAND_READY
  │ 2. Write command bytes to DataFifo (paced by BurstCount)
  │ 3. Set TIS_PC_STS_GO to start execution
  │ 4. Poll STS for DataAvail (90 second timeout)
  │ 5. Read response header from DataFifo
  │ 6. Read remaining response bytes (paced by BurstCount)
  │ 7. Set TIS_PC_STS_READY (return to idle)
  ▼
MMIO to 0xFED40000 (QEMU TIS device)
  │
  ▼
QEMU tpm-tis device ──── Unix socket ──── swtpm process
```

## PCDs Reference

### Required PCDs (set when TPM_ENABLE=TRUE)

| PCD | Value | Type | Purpose |
| ----- | ------- | ------ | --------- |
| `PcdTpmBaseAddress` | `0xFED40000` (package default) | DynamicDefault | TPM TIS/CRB MMIO base address |
| `PcdTpm2HashMask` | `0x02` | DynamicDefault | Hash algorithm filter (SHA256 only) |
| `PcdTpmInstanceGuid` | `gEfiTpmDeviceInstanceTpm20DtpmGuid` | DynamicDefault | Selects discrete TPM 2.0 device type (set by Tcg2ConfigPei at runtime) |
| `PcdTpm2AcpiTableRev` | `4` | DynamicHii | ACPI TPM2 table revision |
| `PcdUserPhysicalPresence` | `FALSE` | FixedAtBuild | No physical user presence assertion |

### Memory Type PCDs

| PCD | Value (pages) | Purpose |
| ----- | --------------- | --------- |
| `PcdMemoryTypeEfiACPIReclaimMemory` | `0x2B` (43) | Includes TPM ACPI tables |
| `PcdMemoryTypeEfiACPIMemoryNVS` | `0x80` (128) | ACPI NVS memory |
| `PcdMemoryTypeEfiReservedMemoryType` | `0x510` | Reserved memory |
| `PcdMemoryTypeEfiRuntimeServicesCode` | `0x100` | Runtime code |
| `PcdMemoryTypeEfiRuntimeServicesData` | `0x700` | Runtime data |

### Conditional PCDs (TPM_CONFIG_ENABLE=TRUE)

| PCD | Value | Purpose |
| ----- | ------- | --------- |
| `PcdTcgPhysicalPresenceInterfaceVer` | `"1.3"` | TCG PPI specification version reported to OS |
