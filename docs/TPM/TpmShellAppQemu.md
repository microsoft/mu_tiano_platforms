# TpmShellApp on QEMU Platforms

Platform-specific guidance for running
[TpmShellApp](https://github.com/microsoft/mu_basecore/blob/release/202511/SecurityPkg/Applications/TpmShellApp/TpmShellApp.md)
on the QEMU Q35 and SBSA platforms.

## Table of Contents

- [FDF Placement](#fdf-placement)
- [Protocol Availability](#protocol-availability)
- [Running on QEMU SBSA](#running-on-qemu-sbsa)
- [Running on QEMU Q35](#running-on-qemu-q35)
- [Hash Algorithm Configuration](#hash-algorithm-configuration)
- [Expected Behavior with MinimumLib](#expected-behavior-with-minimumlib)
- [Event Log and Replay](#event-log-and-replay)
- [Findings](#findings)

## FDF Placement

On SBSA, add the INF to `FV.FvMain`. On Q35, add it to `FV.DXEFV`:

```ini
INF SecurityPkg/Applications/TpmShellApp/TpmShellApp.inf
```

## Protocol Availability

The `EFI_TCG2_PROTOCOL` is installed by `Tcg2Dxe.efi`, which only loads when
`TPM_ENABLE=TRUE` (Q35) or `TPM2_ENABLE=TRUE` (SBSA).

## Running on QEMU SBSA

On SBSA, the TpmShellApp is placed directly in the firmware volume, which is mapped as an
`FSx:` device in the UEFI shell. Build with:

```bash
stuart_build -c Platforms/QemuSbsaPkg/PlatformBuild.py --FlashRom \
  BLD_*_TPM2_ENABLE=TRUE \
  TPM_DEV=/tmp/mytpm1/swtpm-sock
```

At the UEFI shell, run:

```text
Shell> TpmShellApp.efi help
Shell> TpmShellApp.efi info
Shell> TpmShellApp.efi eventlog
Shell> TpmShellApp.efi replay
```

Or navigate to the correct FS mapping first:

```text
Shell> map -r
Shell> FS0:
FS0:\> TpmShellApp.efi info
```

## Running on QEMU Q35

On Q35, shell applications are loaded from a virtual drive (FAT filesystem image), not
directly from the firmware volume. The `FILE_REGEX` build parameter controls which
binaries are copied to the virtual drive:

```bash
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py --FlashRom \
  BLD_*_TPM_ENABLE=TRUE \
  TPM_DEV=/tmp/mytpm1/swtpm-sock \
  FILE_REGEX=TpmShellApp.efi
```

At the UEFI shell, find the virtual drive's FS mapping (typically backed by a PCI device,
not `Fv(...)`) and run:

```text
Shell> map -r
Shell> FS0:
FS0:\> TpmShellApp.efi info
```

If the app is not found, verify the virtual drive was created with the binary:

```text
Shell> FS0:
FS0:\> dir
```

## Hash Algorithm Configuration

Both QEMU platforms set `PcdTpm2HashMask` to `0x02` (SHA256 only). This means the app
will typically only see SHA256 as supported and active, even though swtpm may support all
five algorithms. To see additional algorithms, update `PcdTpm2HashMask` in the platform
DSC.

## Expected Behavior with MinimumLib

Both QEMU platforms use `DxeTcg2PhysicalPresenceMinimumLib` when TPM is enabled. This
library has specific behaviors that affect TpmShellApp results:

### `setpcr` with Already-Active Banks

When the requested banks match the currently active banks, `Tcg2Dxe` sends
`TCG2_PHYSICAL_PRESENCE_NO_ACTION` to the PP library. MinimumLib processes `NO_ACTION`
successfully, returning `EFI_SUCCESS`.

```text
Shell> TpmShellApp setpcr 0x2    (SHA256 already active)
Submitting SetActivePcrBanks with parameter 2
Status: Success
Request submitted. Changes will take effect after reboot.
```

### `setpcr` with Different Banks

When the requested banks differ from active banks, `Tcg2Dxe` sends
`TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS` to the PP library. MinimumLib rejects this
operation as it only supports Clear operations.

```text
Shell> TpmShellApp setpcr 0x4    (SHA384, not currently active)
Submitting SetActivePcrBanks with parameter 4
Status: Unsupported
SetActivePcrBanks failed.
```

This is **expected and correct behavior** — MinimumLib is designed to block PCR bank
changes.

### `logall` Expected Behavior

If the platform only has SHA256 registered (default), `logall` requests the same bank
that's already active, resulting in a `NO_ACTION` → `EFI_SUCCESS`. If additional
algorithms were registered, it would request banks different from the active set,
triggering a `SET_PCR_BANKS` rejection.

### `lastresponse` Expected Behavior

Reports the result stored in the `Tcg2PhysicalPresence` NV variable by
`ProcessRequest`. If no prior `SetActivePcrBanks` was processed through a reboot cycle,
the response will show no operation present.

```text
Shell> TpmShellApp lastresponse
  Operation present: NO
  Response code:     0
```

## Event Log and Replay

### `eventlog` on QEMU

The `eventlog` command dumps the crypto-agile TCG2 event log. On QEMU with the default
SHA256-only configuration, each event contains a single SHA256 digest.

```text
Shell> TpmShellApp.efi eventlog

[TCG2 Event Log]

Spec ID Event: 1 algorithm(s)
  SHA256 (alg 0xb, digest 32 bytes)

Event 1: PCR 0 EV_S_CRTM_VERSION (0x8)
  SHA256: ab cd ef ...
  Event data: 2 bytes
Event 2: PCR 0 EV_EFI_PLATFORM_FIRMWARE_BLOB (0x80000008)
  SHA256: 01 23 45 ...
  Event data: 16 bytes
...
Total: N event(s)
```

If `PcdTpm2HashMask` is updated to enable additional algorithms (e.g., `0x06` for
SHA256 + SHA384), each event will contain multiple digests — one per active algorithm.

### `replay` on QEMU

The `replay` command replays extend operations from the event log to compute expected
PCR values, then reads the actual PCR values from the TPM via `SubmitCommand` and
compares them.

```text
Shell> TpmShellApp.efi replay

[Event Log Replay]

Replayed N event(s) across 1 algorithm(s).

PCR 0:
  SHA256 Replayed: aa bb cc ...
  SHA256 Actual:   aa bb cc ...
  Result: PASS

PCR 1:
  SHA256 Replayed: 11 22 33 ...
  SHA256 Actual:   11 22 33 ...
  Result: PASS

...
Summary: M PCR bank(s) verified, M PASS, 0 FAIL
```

On a freshly booted QEMU platform with swtpm, all PCRs should show `PASS`. A `FAIL`
indicates that either the event log is incomplete (e.g., truncated) or an extend
operation occurred outside the logged event flow.

### Replay with Multiple Algorithms

When multiple hash algorithms are active, the replay verifies each algorithm
independently. Algorithms not supported by `BaseCryptLib` (e.g., SM3_256) are skipped
with a message:

```text
PCR 0:
  SHA256 Replayed: ...
  SHA256 Actual:   ...
  Result: PASS
  SM3_256: (replay not supported, skipped)
```

### Truncated Event Log

If the firmware's event log buffer was exhausted, the `eventlog` and `replay` commands
print a warning:

```text
Warning: Event log was truncated.
```

For `replay`, a truncated log means the replayed PCR values will not include all
extensions, so mismatches are expected.

## Findings

**April 2026:**

Manually updating Tpm2HashMask in the platform .dsc is dangerous. On Q35, PEI will
pick up the change and auto enable/disable any PCR banks that differ from the current
active banks in the TPM. (See Tcg2Pei - SyncPcrAllocationsAndPcrMask) If the platform
also disables (i.e. removes) any of the supported hashing algorithms from the platform
.dsc there are no safeguards to prevent the TPM from enabling/disabling any active banks
based on platform support. Because of this there can be active banks that will show up
as active when querying the TPM but will not show up as active in the Tcg2Protocol call.

Example:

- Tpm2HashMask is updated to 0x06 (i.e. SHA256 + SHA384 supported).
- SHA384 support is removed from Tcg2Pei and Tcg2Dxe.
- Build/Run the Q35 platform.
- TPM reports SHA256 as the only active PCR bank.
- Tcg2Pei recognizes there is a mismatch between platform support (i.e. Tpm2HashMask)
  and TPM active banks. Activates SHA384 into a ResetCold.
- TPM reports SHA256 + SHA384 as active PCR banks.
- HashLibCryptoRouterPei and HashLibCryptoRouterDxe set PcdTcg2HashAlgorithmBitmap based on
  successfully registered hash algorithms.
- Tcg2Pei registers SHA256 but is unable to register SHA384.
- Tcg2Dxe registers SHA256 but is unable to register SHA384.
- Tcg2Dxe sets local variables based on what the TPM reports and PcdTcg2HashAlgorithmBitmap.

  ```C
  mTcgDxeData.BsCap.HashAlgorithmBitmap = TpmHashAlgorithmBitmap & PcdGet32 (PcdTcg2HashAlgorithmBitmap);
  mTcgDxeData.BsCap.ActivePcrBanks      = ActivePCRBanks & PcdGet32 (PcdTcg2HashAlgorithmBitmap);
  ```

- Tcg2Protocol is installed with only SHA256 support reported even though SHA384 is active.
