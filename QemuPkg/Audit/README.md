# QEMU Platform Audits

This directory contains platform-owned audit applications and their execution
and evidence tooling for the QEMU reference platforms.

## Secure Boot Key ECIT audit

[SecureBootKeyAudit](SecureBootKeyAudit/README.md) performs destructive Secure
Boot key and image-verification operations to validate ECIT claims. It supports
the QemuQ35 developer workflow and a physical-machine workflow for dedicated,
recoverable test systems.

> [!CAUTION]
> The Secure Boot audit changes PK, `db`, and `dbx`, and requests a cold reset.
> Run it only on a non-production test system with a proven recovery path and a
> complete variable-state backup.

### Build the standalone application

From the `mu_tiano_platforms` root, use the audit-only QemuPkg build:

```bash
stuart_ci_build \
  -c .pytool/CISettings.py \
  -p QemuPkg \
  -a X64 \
  -t DEBUG \
  TOOL_CHAIN_TAG=GCC5 \
  BLD_*_SECURE_BOOT_KEY_AUDIT_ONLY=TRUE
```

The standalone application is written to:

```text
Build/QemuPkg/DEBUG_GCC5/X64/SecureBootKeyAuditApp.efi
```

See:

- [SecureBootKeyAudit/README.md](SecureBootKeyAudit/README.md) for the
  physical-machine workflow and evidence contract.
- [SecureBootKeyAudit/QEMU.md](SecureBootKeyAudit/QEMU.md) for the guarded
  QEMU workflow.
