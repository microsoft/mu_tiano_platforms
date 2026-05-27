# TPM on QEMU Q35

This document describes the TPM 2.0 architecture for the QEMU Q35 platform. Q35 uses a
direct CRB/FIFO path between firmware and the TPM device.

## Table of Contents

- [Build Configuration](#build-configuration)
- [Platform Memory Layout](#platform-memory-layout)
- [Architecture Overview](#architecture-overview)
- [TPM Device Library Stack](#tpm-device-library-stack)
- [Hash Library Architecture](#hash-library-architecture)
- [Physical Presence Interface](#physical-presence-interface)
- [swtpm Setup](#swtpm-setup)
- [Communication Flow](#communication-flow)
- [PCDs Reference](#pcds-reference)

## Build Configuration

The TPM is disabled by default. To enable it, pass the build define and provide the swtpm
socket path or provide it in a BuildConfig.conf file placed at the root level of the repo:

```bash
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py --FlashRom \
  BLD_*_TPM_ENABLE=TRUE \
  TPM_DEV=/tmp/mytpm1/swtpm-sock
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
┌──────────────────────────────────────────────────────────────────────────┐
│ UEFI Firmware (x86_64)                                                   │
│                                                                          │
│  ┌─── PEI Phase ──────────────────────────────────────────────────────┐  │
│  │                                                                    │  │
│  │  Tcg2ConfigPei                                                     │  │
│  │    │ Detect TPM 1.2 vs 2.0 at 0xFED40000                           │  │
│  │    │ Set PcdTpmInstanceGuid accordingly                            │  │
│  │    ▼                                                               │  │
│  │  Tcg2Pei                                                           │  │
│  │    │ Tpm2Startup(TPM_SU_CLEAR)                                     │  │
│  │    │ SyncPcrAllocationsAndPcrMask()                                │  │
│  │    │ Tpm2SelfTest()                                                │  │
│  │    │ Measure firmware volumes (CRTM) into PCR[0-7]                 │  │
│  │    │ Uses Tpm2DeviceLibDTpm (direct MMIO)                          │  │
│  │    ▼                                                               │  │
│  │  Install TpmInitializedPpi                                         │  │
│  └────────────────────────────────────────────────────────────────────┘  │
│                                                                          │
│  ┌─── DXE Phase ──────────────────────────────────────────────────────┐  │
│  │                                                                    │  │
│  │  Tcg2Dxe                                                           │  │
│  │    │ Uses Tpm2DeviceLibRouter → Tpm2InstanceLibDTpm (direct MMIO)  │  │
│  │    │ Registers hash algorithms via HashLibBaseCryptoRouter         │  │
│  │    ▼                                                               │  │
│  │  Installs EFI_TCG2_PROTOCOL                                        │  │
│  └────────────────────────────────────────────────────────────────────┘  │
│                                                                          │
│  ┌─── BDS Phase ──────────────────────────────────────────────────────┐  │
│  │                                                                    │  │
│  │  DeviceBootManagerAfterConsole                                     │  │
│  │    │ Tcg2PhysicalPresenceLibProcessRequest (NULL)                  │  │
│  │    │ Process any pending PP request before shell launch            │  │
│  │    ▼                                                               │  │
│  │  Creates TCG2_PHYSICAL_PRESENCE_VARIABLE if it doesn't exist       │  │
│  └────────────────────────────────────────────────────────────────────┘  │
│                                                                          │
│  ┌─── UEFI Shell ─────────────────────────────────────────────────────┐  │
│  │                                                                    │  │
│  │  UEFI Shell / OS / TpmShellApp                                     │  │
│  │    │ gBS->LocateProtocol(&gEfiTcg2ProtocolGuid)                    │  │
│  │    │ Tcg2Protocol->GetCapability / SetActivePcrBanks / etc.        │  │
│  └────┼───────────────────────────────────────────────────────────────┘  │
│       ▼                                                                  │
│  Tpm2DeviceLibDTpm ─── direct MMIO reads/writes to 0xFED40000            │
│       │                                                                  │
├───────┼──────────────────────────────────────────────────────────────────┤
│       ▼                                                                  │
│  QEMU TPM TIS device (-device tpm-tis,tpmdev=tpm0)                       │
│       │                                                                  │
│       ▼                                                                  │
│  Unix socket ──────── swtpm process (--tpm2)                             │
└──────────────────────────────────────────────────────────────────────────┘
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

If the TPM presents a CRB interface (as opposed to TIS/FIFO), the register layout per
the TCG PC Client Platform TPM Profile (PTP) specification:

| Offset | Register | Description |
| -------- | ---------- | ------------- |
| `0x00` | `LocalityState` | Current locality ownership |
| `0x08` | `LocalityControl` | Request access, relinquish, seize |
| `0x0C` | `LocalityStatus` | Granted / been seized status |
| `0x30` | `InterfaceId` | CRB vs FIFO detection, version, VID/DID |
| `0x40` | `CrbControlRequest` | `cmdReady` (BIT0), `goIdle` (BIT1) |
| `0x44` | `CrbControlStatus` | `tpmIdle` (BIT0), `tpmSts` (BIT1) |
| `0x48` | `CrbControlCancel` | Write 1 to cancel in-progress command |
| `0x4C` | `CrbControlStart` | Write 1 to begin command execution |
| `0x58` | `CrbControlCommandSize` | Command buffer size (typically `0xF80`) |
| `0x5C` | `CrbControlCommandAddressLow` | Low 32 bits of command buffer address |
| `0x60` | `CrbControlCommandAddressHigh` | High 32 bits of command buffer address |
| `0x64` | `CrbControlResponseSize` | Response buffer size (typically `0xF80`) |
| `0x68` | `CrbControlResponseAddress` | Low 64 bits of response buffer address |
| `0x80` | `CrbDataBuffer[0xF80]` | 3968-byte shared command/response buffer |

### TIS Register Layout

QEMU's `tpm-tis` device presents a TIS (TPM Interface Specification) / FIFO interface.
The key registers:

| Offset | Register | Description |
| -------- | ---------- | ------------- |
| `0x00` | `Access` | Locality access control |
| `0x08` | `IntEnable` | Interrupt enable |
| `0x10` | `IntVector` | Interrupt vector |
| `0x18` | `STS` | Status (commandReady, tpmGo, dataAvail, burstCount) |
| `0x24` | `DataFifo` | Data I/O register (write commands, read responses) |
| `0xF00` | `Vid` | Vendor ID |
| `0xF04` | `Did` | Device ID |
| `0xF08` | `Rid` | Revision ID |

The TIS flow uses `BurstCount` from the STS register to pace reads and writes through
the `DataFifo` register, one burst at a time.

## Hash Library Architecture

Tcg2Dxe and Tcg2Pei both use `HashLibBaseCryptoRouter` with all five hash instance
libraries linked:

```ini
# Per-module library overrides for Tcg2Dxe and Tcg2Pei
HashLib|SecurityPkg/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterDxe.inf
NULL|SecurityPkg/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
NULL|SecurityPkg/Library/HashInstanceLibSha384/HashInstanceLibSha384.inf
NULL|SecurityPkg/Library/HashInstanceLibSha512/HashInstanceLibSha512.inf
NULL|SecurityPkg/Library/HashInstanceLibSm3/HashInstanceLibSm3.inf
```

### Registration Flow

1. `HashLibBaseCryptoRouterConstructor` resets `PcdTcg2HashAlgorithmBitmap` to 0.
2. Each `HashInstanceLib` constructor calls `RegisterHashInterfaceLib()`.
3. `RegisterHashInterfaceLib()` checks the algorithm against `PcdTpm2HashMask` (`0x02` =
   SHA256 only). Algorithms not in the mask return `EFI_UNSUPPORTED`.

### Hash Algorithm Bitmask Values

| Algorithm | Bit | Value |
| ----------- | ----- | ------- |
| SHA1 | BIT0 | `0x01` |
| SHA256 | BIT1 | `0x02` |
| SHA384 | BIT2 | `0x04` |
| SHA512 | BIT3 | `0x08` |
| SM3_256 | BIT4 | `0x10` |

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

`DeviceBootManagerAfterConsole()` in `DeviceBootManagerLibQemu` handles PP processing
before the shell launches:

```c
if (BootMode != BOOT_ON_FLASH_UPDATE) {
    // 1. GUI-based PP prompt (if TPM_PP_PROTOCOL is available)
    Status = gBS->LocateProtocol (&gTpmPpProtocolGuid, NULL, (VOID **)&TpmPp);
    if (!EFI_ERROR (Status) && (TpmPp != NULL)) {
        TpmPp->PromptForConfirmation (TpmPp);
    }

    // 2. Standard TCG2 PP processing
    Tcg2PhysicalPresenceLibProcessRequest (NULL);
}
```

`ProcessRequest` reads the `Tcg2PhysicalPresence` NV variable (creating it with zeroed
contents if it doesn't exist), executes any pending request, and stores the result for
later retrieval via `ReturnOperationResponseToOsFunction()`.

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

Create the TPM state directory and start swtpm:

```bash
mkdir -p /tmp/mytpm1
swtpm socket \
  --tpmstate dir=/tmp/mytpm1 \
  --ctrl type=unixio,path=/tmp/mytpm1/swtpm-sock \
  --tpm2 \
  --log level=20
```

### Automatic Setup (QemuRunner)

When the `TPM_DEV` environment variable is set, `QemuRunner.py` automatically starts
swtpm in a background thread before launching QEMU:

```python
@staticmethod
def RunThread(env):
    tpm_path = env.GetValue("TPM_DEV")
    tpm_cmd = "swtpm"
    tpm_args = (
        f"socket --tpmstate dir={'/'.join(tpm_path.rsplit('/', 1)[:-1])} "
        f"--ctrl type=unixio,path={tpm_path} --tpm2 --log level=20"
    )
    utility_functions.RunCmd(tpm_cmd, tpm_args)
```

The thread is launched before QEMU starts and joined after QEMU exits.

### QEMU Arguments

The `QemuCommandBuilder.with_tpm()` method adds three arguments for Q35:

```text
-chardev socket,id=chrtpm,path=/tmp/mytpm1/swtpm-sock
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
  │   ↳ MinimumLib: rejects SET_PCR_BANKS → returns EFI_UNSUPPORTED
  │   ↳ MinimumLib: NO_ACTION (already-active) → writes NV variable → EFI_SUCCESS
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
