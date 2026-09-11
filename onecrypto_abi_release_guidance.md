# OneCrypto ABI and Release Guidance

This document defines how prototype OneCrypto protocol changes are coordinated
between `mu_basecore`, `mu_crypto_release`, and platform consumers.

## ABI policy

- A protocol major version identifies an incompatible binary function-table
  contract. Consumers must reject providers with a different major version.
- A protocol minor version is append-only within a major version. A consumer may
  require a minimum minor version for a service introduced during that major
  version.
- During prototype development, use a new major version when the function table
  or compatibility policy is intentionally being reworked. Do not preserve a
  misleading compatibility promise solely because an older prototype binary
  exists.
- A provider and `BaseCryptLibOnOneCrypto` consumer must use the same protocol
  header revision for a protocol-major transition.

## Release train policy

An ABI-major change is a coordinated release train, not an independently
mergeable BaseCryptLib change.

1. Review the BaseCryptLib protocol and facade changes on a dedicated branch.
2. Build a local OneCrypto provider from the same protocol revision.
3. Use a local, uncommitted `file://` ext_dep override to validate the provider
   with a platform consumer.
4. Merge or rebase the reviewed BaseCryptLib API into the OneCrypto feature
   branch and build the candidate provider archive.
5. Publish an immutable OneCrypto release or pre-release archive with its SHA-256.
6. Update the committed BaseCryptLib ext_dep manifest to the published archive,
   version, and SHA-256.
7. Merge the consumer-facing protocol change only after the matching provider
   artifact is available through the committed manifest.

The local override and its copied archive are development tools. They must not
be committed as a substitute for the published dependency pin.

## Validation gate

Before publishing or updating the committed manifest, validate the exact archive
that will be consumed:

1. Build `OneCrypto-Drivers.zip` from the candidate OneCrypto branch.
2. Refresh the platform ext_dep and confirm `extdep_state.yaml` records the
   expected version.
3. Build a QEMU platform using the archive.
4. Run `BaseCryptLibUnitTestApp` and require `PROGRESS - Success` with no failed
   test markers.

For a protocol-major change, the test matrix must also prove that a mismatched
major version is rejected before a protocol service pointer is called.

## Pull request policy

- Protocol, facade, provider, and dependency-manifest changes should be split
  into reviewable commits by ownership package.
- Stacked pull requests must state their merge dependency. A BaseCryptLib PR
  that requires an unpublished provider archive is reviewable but must not merge
  alone.
- The committed manifest must name an immutable artifact. Do not use a mutable
  branch asset, a local path, or a checksum that was not produced from the
  reviewed provider revision.
