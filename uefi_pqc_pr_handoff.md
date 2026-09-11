# UEFI PQC Code First PR handoff

This is a public-source handoff list for the Tianocore `PQC Code First` project. It is intended to help with implementation planning and cherry-picking, not as a confirmed UEFI 2.12 release list.

Important note: the project description explicitly says the work is exploratory and not guaranteed to become the final UEFI specification. The official public UEFI Forum spec as of 2026-09-09 remains 2.11.

## Legend

- Spec PR: proposed text change in `UEFI-Specification-Release`
- Implementation PR: code or staging changes in `edk2` / `edk2-staging`
- Status: open / merged / rejected
- Best use: `implement`, `cherry-pick`, or `monitor`

## Recommended implementation buckets

### 1) Secure Boot multi-algorithm validation and policy

| Issue | Title | Spec PR(s) | Implementation PR(s) | Best use |
| --- | --- | --- | --- | --- |
| edk2#12107 | Clarify Multiple signatures verification | jyao1/UEFI-Specification-Release#1 | tianocore/edk2-staging#522 (merged) | implement / cherry-pick |
| edk2#12455 | Reject unsupported algorithm or key size in PK/KEK/DB/DBX | jyao1/UEFI-Specification-Release#9 | — | implement |
| edk2#12526 | Clarify multiple signatures verification order | jyao1/UEFI-Specification-Release#15 | — | implement |
| edk2#12521 | Add missing `EFI_CERT_SHAxxx_GUID` to Authorization Process | jyao1/UEFI-Specification-Release#10 | — | implement |
| edk2#12527 | For DBX, deprecate `EFI_CERT_X509_GUID`; prefer TBS cert | jyao1/UEFI-Specification-Release#16 | — | implement |
| edk2#12528 | Mandate one `EFI_SIGNATURE_LIST` per `SetVariable()` call | jyao1/UEFI-Specification-Release#17 | — | implement |
| edk2#12405 | Add `TBSCertificate` (`EFI_CERT_X509_SHAxxx`) usage for `db` | jyao1/UEFI-Specification-Release#5 | — | implement |
| edk2#12524 | Remove language that limits crypto agility | jyao1/UEFI-Specification-Release#13 | — | monitor |

### 2) Variable authentication and signed data compatibility

| Issue | Title | Spec PR(s) | Implementation PR(s) | Best use |
| --- | --- | --- | --- | --- |
| edk2#12108 | Clarify number of `SignerInfo` in `AuthVariable` update | jyao1/UEFI-Specification-Release#2 | — | implement |
| edk2#12159 | Allow KEK self-signed append operations | jyao1/UEFI-Specification-Release#4 | — | implement |
| edk2#12523 | Remove `EFI_VARIABLE_AUTHENTICATION_3` descriptor | jyao1/UEFI-Specification-Release#12 | — | implement |
| edk2#12407 | Deprecate Private Authenticated Variable Support | jyao1/UEFI-Specification-Release#7 | — | implement |
| edk2#12541 | Remove `dbt`, RFC 3161, and clarify `TimeOfRevocation` | spbrogan/UEFI-Specification-Release#2 | tianocore/edk2-staging#540, #545, #546 (merged) | implement / cherry-pick |
| edk2#12574 | `db`/`dbx` formats include unused or bug-prone fields | jyao1/UEFI-Specification-Release#18 | — | monitor |

### 3) CMS / PKCS#7 / verification semantics

| Issue | Title | Spec PR(s) | Implementation PR(s) | Best use |
| --- | --- | --- | --- | --- |
| edk2#12561 | Deprecate PKCS wording and align `EFI_PKCS7_VERIFY_PROTOCOL` with CMS | Flickdm/UEFI-Specification-Release#1 | — | implement |
| edk2#12706 | `PKCS7` protocol clarification | jyao1/UEFI-Specification-Release#19 | — | implement |
| edk2#12109 | Clarify certificate expiry and system time usage | jyao1/UEFI-Specification-Release#3 | — | implement |
| edk2#12406 | Add hash deprecation text | jyao1/UEFI-Specification-Release#6 | — | implement |

### 4) Capability reporting and firmware visibility

| Issue | Title | Spec PR(s) | Implementation PR(s) | Best use |
| --- | --- | --- | --- | --- |
| edk2#12408 | Introduce `EFI_CRYPTO_INDICATOR_TABLE` (ECIT) | jyao1/UEFI-Specification-Release#8 | — | implement |
| edk2#12522 | Deprecate `SignatureSupport` variable | jyao1/UEFI-Specification-Release#11 | — | implement |

### 5) Legacy cleanup and docs simplification

| Issue | Title | Spec PR(s) | Implementation PR(s) | Best use |
| --- | --- | --- | --- | --- |
| edk2#12525 | Remove Audit and Deployed Mode | Flickdm/UEFI-Specification-Release#2; jyao1/UEFI-Specification-Release#14 | — | implement |
| edk2#12688 | Remove Image Execution Information Table | Flickdm/UEFI-Specification-Release#2 | — | implement |

## Full issue-to-PR index

### In progress / active

- edk2#12107 — Clarify Multiple signatures verification  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/1  
  - Impl PR: https://github.com/tianocore/edk2-staging/pull/522  
  - Status: spec open, staging merged

- edk2#12108 — Clarify number of `SignerInfo` in `AuthVariable` update  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/2

- edk2#12109 — Clarify certificate expiry and the use of the system time  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/3

- edk2#12159 — Allow KEK self-signed append operations  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/4

- edk2#12405 — Add `TBSCertificate` (`EFI_CERT_X509_SHAxxx`) usage for `db`  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/5

- edk2#12406 — Add hash deprecation text  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/6

- edk2#12407 — Deprecate Private Authenticated Variable Support  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/7

- edk2#12408 — Introduce `EFI_CRYPTO_INDICATOR_TABLE` (ECIT)  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/8

- edk2#12455 — Reject unsupported algorithm or key size in PK/KEK/DB/DBX  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/9

- edk2#12521 — Add missing `EFI_CERT_SHAxxx_GUID` to Authorization Process  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/10

- edk2#12522 — Deprecate `SignatureSupport` variable  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/11

- edk2#12523 — Remove `EFI_VARIABLE_AUTHENTICATION_3` descriptor  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/12

- edk2#12524 — Remove language that limits Crypto Agility  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/13

- edk2#12525 — Remove Audit and Deployed Mode  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/14  
  - Spec PR: https://github.com/Flickdm/UEFI-Specification-Release/pull/2

- edk2#12526 — Clarify multiple signatures verification order  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/15

- edk2#12527 — For DBX, deprecate `EFI_CERT_X509_GUID`; recommend TBS cert  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/16

- edk2#12528 — Mandate one `EFI_SIGNATURE_LIST` per `SetVariable()` call  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/17

- edk2#12541 — Remove `dbt`, RFC 3161, and clarify `TimeOfRevocation`  
  - Spec PR: https://github.com/spbrogan/UEFI-Specification-Release/pull/2  
  - Impl PRs: https://github.com/tianocore/edk2-staging/pull/540, https://github.com/tianocore/edk2-staging/pull/545, https://github.com/tianocore/edk2-staging/pull/546

- edk2#12561 — Deprecate PKCS wording and align `EFI_PKCS7_VERIFY_PROTOCOL` with CMS  
  - Spec PR: https://github.com/Flickdm/UEFI-Specification-Release/pull/1

- edk2#12574 — `db`/`dbx` data formats include unused and bug-prone fields  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/18

- edk2#12688 — Remove Image Execution Information Table  
  - Spec PR: https://github.com/Flickdm/UEFI-Specification-Release/pull/2

- edk2#12706 — `PKCS7` protocol clarification  
  - Spec PR: https://github.com/jyao1/UEFI-Specification-Release/pull/19

### Rejected / not recommended

- edk2#12110 — Add recommendation on how to switch to PQC only UEFI firmware  
  - Closed PR: https://github.com/tianocore/edk2/pull/12127  
  - Recommendation: do not include in the implementation checklist

## Cherry-pick guidance

If the goal is to cherry-pick from public code instead of implementing from scratch, the best concrete candidates are:

1. `edk2#12107` + `tianocore/edk2-staging#522`  
   Best first patch for multi-signature validation semantics.

2. `edk2#12541` + `tianocore/edk2-staging#540`, `#545`, `#546`  
   Best example of a spec + staging change set already landed together.

3. `edk2#12525` + spec cleanup for Audit/Deployed Mode and IEIT removal  
   Useful if you want to simplify and remove stale behavior.

4. `edk2#12408` and `edk2#12522`  
   Best for capability reporting design work; likely the most strategic long-term feature.

## Suggested ordering

1. Multi-signature validation and algorithm rejection
2. Variable-auth and certificate format cleanup
3. CMS/PKCS#7 semantic alignment
4. ECIT capability reporting
5. Legacy cleanup and deprecation pass

## Current branch implementation checklist

Historical review target: `MU_BASECORE` branch
`dev/202511/post-quantum-staging` at `506bbaedde`, compared with merge base
`7cff1f44f9` from `origin/release/202511` (27 commits; 63 files changed).
Current integration target: `dev/pqc/cherry-picks` at `06edd92c57`, rebased on
upstream PQC staging through `cf7d50e824`. `[x]` means the branch contains a
meaningful implementation for the handoff item, `[-]` means enabling or related
work is present but the proposal is not fully implemented, `[ ]` means no
implementation was found in this branch, and `[~]` means the work exists on a
separate local remote branch but is not merged here.

### 1) Secure Boot multi-algorithm validation and policy

- [-] `edk2#12107` / `edk2#12526` - The rewritten
  `DxeImageVerificationLib2` evaluates signature databases and publishes a
  detailed image-verification result table, with host tests. It is strong
  validation infrastructure, but this review did not identify the proposed
  multi-signature verification-order policy as a discrete implementation.
- [-] `edk2#12455` - Database parsing skips unknown signature types and
  malformed signature-list entries, and the new crypto APIs carry algorithm
  GUIDs. Explicit PK/KEK/db/dbx rejection of unsupported algorithms and key
  sizes is not implemented in the authenticated-variable update path.
- [-] `edk2#12521` - V2 image-authentication definitions and
  `EFI_CERT_X509_SHA*` / TBS-certificate hash processing are present; the
  authorization-process language and all SHA certificate GUID cases have not
  been implemented as a standalone policy change.
- [-] `edk2#12527` - The image verifier supports TBS-certificate hash entries
  for database lookup. `EFI_CERT_X509_GUID` handling remains, so dbx X.509
  deprecation has not landed.
- [ ] `edk2#12528` - One-`EFI_SIGNATURE_LIST`-per-`SetVariable()` enforcement
  was not found.
- [x] `edk2#12405` - Implemented through `EFI_CERT_X509_SHA*` TBS hash lookup
  and matching in `DxeImageVerificationLib2`, supported by
  `X509GetTbsCertHash`, V2 structures, and unit tests.
- [ ] `edk2#12524` - No direct crypto-agility specification cleanup was found.

### 2) Variable authentication and signed-data compatibility

- [x] `edk2#12108` - MU_BASECORE implements the AuthVariable `SignerInfo`
  cardinality policy with committed changes `555b7a9c7a` and `e099ddf845`:
  time-based updates require `CmsGetSignerInfoNum(SigData, SigDataSize) == 1`.
  The matching OneCrypto OpenSSL provider and protocol registration are
  committed as `86750c8569` and `b8623aee2e` in `mu_crypto_release`.
  A copied local AARCH64 drop passes the QemuArmVirtPkg BaseCryptLib suite
  (98 tests, 0 failures, 0 errors). Direct BaseCryptLib tests verify the
  OneCrypto provider reports one signer for real PKCS#7 signed data and zero
  for invalid data. AuthVariableLib GoogleTest coverage rejects zero and
  multiple `SignerInfo` values before PKCS#7 verification, and proves one
  `SignerInfo` proceeds beyond the cardinality check. A full positive signed
  authenticated-variable update test and a direct multi-signer CMS fixture
  remain outstanding.
- [x] `edk2#12159` - Core KEK append behavior from
  `tianocore/edk2-staging#528` has been cherry-picked as local commit
  `69a1e2536d` from upstream commit `52fffc8364dc`: KEK updates still try PK
  authorization first, and append writes fall back to KEK authorization through
  `ProcessVarWithKek()` if PK authorization fails. Upstream EmulatorPkg test
  assets were not present in MU_BASECORE. The adjacent MLDSA `messageDigest`
  patch is not needed for the current OneCrypto OpenSSL/CMS verification path
  because `CMS_verify()` already performs the detached-content digest binding.
- [ ] `edk2#12523` - No removal of
  `EFI_VARIABLE_AUTHENTICATION_3` was found.
- [x] `edk2#12407` - Private authenticated-variable support is removed. The
  AuthVariable certificate policy uses the provider-neutral
  `X509IsPublicKeySupported()` service rather than naming a specific public-key
  algorithm.
- [ ] `edk2#12541` - The branch still contains `dbt` and
  `TimeOfRevocation` handling; it does not include the dbt/RFC 3161 removal
  work.
- [ ] `edk2#12574` - No db/dbx data-format simplification was found.

### 3) CMS / PKCS#7 / verification semantics

- [~] CMS signing API migration - The direct `CmsGetSignerInfoNum()` test uses
  the existing `Pkcs7Sign()` API solely to generate a real one-signer CMS
  SignedData fixture. Migrate legacy `Pkcs7*` APIs to CMS-named contracts with
  equivalent signing support before PQC providers are expected to implement
  them; a `CmsSign()` replacement must support the required signer semantics.
- [-] `edk2#12561` / `edk2#12706` - `AuthenticodeVerifyEx` and the new
  `AuthenticodeLib` improve signed-image verification and parsing. No update to
  `EFI_PKCS7_VERIFY_PROTOCOL` terminology or CMS contract was found.
- [ ] `edk2#12109` - No certificate-expiry/system-time policy change was found.
- [-] `edk2#12406` - `X509GetTbsCertHash` is deprecated and `HashAllByGuid` was
  added, but no broader hash-deprecation policy change was found.

### 4) Capability reporting and firmware visibility

- [~] `edk2#12408` - ECIT is not merged into this branch. It is available for
  integration on `origin/feat/ecit-capability-reporting` and its supporting
  `feat/ecit-*` branches (crypto contract, collector, DXE/MM reporting, and dump
  application).
- [ ] `edk2#12522` - No `SignatureSupport` variable deprecation was found.

### 5) Legacy cleanup and docs simplification

- [ ] `edk2#12525` - The `DxeImageVerificationLib2` rewrite is not an
  implementation of this removal. Both verifiers use
  `IsSecureBootEnabled()`, which reads only `SecureBoot`; `AuditMode` is not
  read by the new verifier, and `DeployedMode` is absent from both the review
  baseline and this branch.
- [ ] `edk2#12688` - No Image Execution Information Table removal was found.

### Included branch work not represented by a handoff checkbox

- [x] OneCrypto external dependency updated to
  `v2.0.0-OneCrypto-PQC-Beta1` by upstream commit `a3ccd1775b`.
- [x] `AuthenticodeLib`, `AuthenticodeVerifyEx`, Authenticode hash helpers,
  trust-anchor recovery, and associated BaseCryptLib tests/mocks/benchmarks.
- [x] Secure Boot image-verification result table, shell test app, V2
  `EFI_CERT_*` structures, rewritten `DxeImageVerificationLib2`, and host-based
  unit tests.
- [-] OneCrypto MLDSA `messageDigest` consistency check from
  `tianocore/edk2-staging#528` commit `7c319aa72f60`. The upstream patch targets
  edk2's custom OpenSSL `BaseCryptLib/Pk` implementation. The current OneCrypto
  OpenSSL path routes `Pkcs7Verify()` through `CmsVerify()` / OpenSSL
  `CMS_verify()`, which already verifies `signedAttributes.messageDigest`
  against the supplied `InData`; keep this open only for non-CMS or MbedTLS
  verification paths if they become production-relevant.

## Authenticated-variable cherry-pick review

Current integration target is `MU_BASECORE/dev/pqc/cherry-picks`, rebased through
upstream commit `cf7d50e824`. The branch still contains `dbt`, `TimeStampDb`,
and RFC3161 timestamp verification. The local port removes `AuthVarTypePriv` and
`certdb` / `certdbv` private authenticated-variable support. No
`EFI_VARIABLE_AUTHENTICATION_3` or
`ENHANCED_AUTHENTICATED_ACCESS` source references were found in the current tree,
so `edk2#12523` appears to be specification-only for this codebase unless a
non-source artifact still carries the descriptor.

### Implementation PRs found

- `tianocore/edk2-staging#527` - `edk2#12407`, private authenticated-variable
  removal. Removes the `AuthVarTypePriv` path, `certdb` / `certdbv` handling,
  private-auth service declarations, and related declarations/tests. This looks
  sufficient for the code side of the spec intent, but it is a compatibility
  removal and should be validated with authenticated-variable negative tests.
  Do not merge the current upstream SecurityPkg code into production unchanged:
  its `MlDsaGetPublicKeyFromX509()` check hard-codes one PQC algorithm in
  AuthVariableLib. Replace it with a provider-neutral X.509 subject-public-key
  capability interface before production integration. The local port now does
  so through `X509IsPublicKeySupported()`, added as an append-only OneCrypto
  protocol v1.3 service with OpenSSL and Mbed TLS provider implementations.
- `tianocore/edk2-staging#528` - `edk2#12159`, allow KEK append authorized by
  KEK. The core production change has been cherry-picked locally as
  `69a1e2536d` from upstream commit `52fffc8364dc`: KEK updates still attempt PK
  authorization first, and append writes fall back to `ProcessVarWithKek()` when
  PK authorization fails. The PR's EmulatorPkg test assets are absent in
  MU_BASECORE. The adjacent MLDSA `messageDigest` patch is not needed for the
  current OneCrypto OpenSSL/CMS verification path because `CMS_verify()` already
  performs the detached-content digest binding.
- `tianocore/edk2-staging#532` - `edk2#12108`, enforce exactly one
  `SignerInfo`. The edk2-staging prototype adds `Pkcs7GetSignerInfoNum()` and
  rejects authenticated-variable payloads where the count is not one. The local
  production port uses a CMS-named OneCrypto/OpenSSL API instead:
  `CmsGetSignerInfoNum()`, exported through OneCrypto protocol version 1.2 and
  consumed by `AuthVariableLib` before any `Pkcs7Verify()` success path.
  MU_BASECORE changes are committed as `555b7a9c7a` and `e099ddf845`; the
  matching OneCrypto provider changes are committed as `86750c8569` and
  `b8623aee2e`.
- `tianocore/edk2-staging#540` - `edk2#12541`, remove `dbt` from variable,
  provisioning, setup UI, measurement, and image-verification flows. This is
  necessary for the spec item but not sufficient by itself because RFC3161 /
  `TimeStampDb` protocol handling is split into `#545`.
- `tianocore/edk2-staging#545` - `edk2#12541`, remove `TimeStampDb` consumption
  from `EFI_PKCS7_VERIFY_PROTOCOL` implementation and deprecate
  `ImageTimestampVerify`. This is required for the RFC3161 half of the spec
  item, but the BaseCryptLib implementation path in MU_BASECORE is OneCrypto, not
  edk2's `BaseCryptLib/Pk` implementation.
- `tianocore/edk2-staging#546` - `edk2#12541`, remove unused `RevocationTime`
  plumbing from the legacy `DxeImageVerificationLib`. This is cleanup, not a full
  spec implementation, and the current branch's `DxeImageVerificationLib2` will
  need a separate review because it still names DB/DBX/DBT helpers in comments
  and support types.

### Direct cherry-pick candidates from dry-run patch checks

- Directly applicable: `2fabc3f01b8e` from `#527`, `aa13895de1c1` from `#532`,
  and these `#540` slices: `ecaa10665a7a`, `ee355cb30532`, `9263e395eed8`,
  `ec54978a516a`, `20f851f865dd`, `afc8207198c6`, plus `ffdba4d129e7` from
  `#545`.
- Needs porting due local divergence: `587e7ede2ba8` (`#527` AuthVariableLib /
  SecurityPkg.dec), `c21a83b5efe8` and `600d441f7573` (`#540` variable runtime
  and AuthVariableLib dbt removal), `a34f986d5bd9` (`#540` `TimeOfRevocation` to
  `Reserved` in a header already changed by the V2 work), and `44b6a0932db0`
  (`#546` old verifier cleanup). `52fffc8364dc` (`#528` KEK append dispatch) was
  in this category and has now been cherry-picked locally.
- Not directly applicable as-is because MU_BASECORE lacks the upstream paths:
  EmulatorPkg/OvmfPkg test and platform files, edk2 `CryptoPkg/Driver`,
  `CryptoPkg/Private/Protocol/Crypto.h`, `BaseCryptLib/Pk`, and MbedTLS crypto
  library files. Equivalent changes must be mapped to OneCrypto protocol/wrapper
  files where needed.

### Sufficiency assessment before cherry-picking

- `edk2#12108` - Core code implementation is now present. The local production
  port intentionally uses `CmsGetSignerInfoNum()` rather than the prototype
  `Pkcs7GetSignerInfoNum()` name because this branch's OpenSSL path verifies via
  `CmsVerify()` / `CMS_verify()`. GoogleTest coverage rejects zero and multiple
  `SignerInfo` values and proves one signer proceeds past the policy check.
  Remaining work is a full positive signed authenticated-variable update test.
- `edk2#12159` - Sufficient narrow behavior has been cherry-picked as a small
  AuthVariableLib change. Remaining gap is validation coverage because the
  upstream shell test assets are absent from this tree.
- `edk2#12407` - Ported from upstream commit `587e7ede2ba8`, with its
  algorithm-specific `MlDsaGetPublicKeyFromX509()` check replaced by the local
  provider-neutral `X509IsPublicKeySupported()` OneCrypto v1.3 service. The
  OpenSSL and Mbed TLS providers independently decide whether the certificate
  subject key supports signatures. AuthVariable GoogleTest coverage confirms a
  private time-based variable is rejected with `EFI_UNSUPPORTED`; existing PK,
  KEK, db, dbx, and dbt authorization paths remain intact.
- `edk2#12541` - Sufficient only as the combined `#540` + `#545` + targeted
  `DxeImageVerificationLib2` cleanup. `#540` alone leaves RFC3161 and
  `TimeStampDb`; `#545` alone leaves `dbt` variables and UI/provisioning paths.
- `edk2#12523` - No code cherry-pick identified; current source appears not to
  define the descriptor already.

Recommended order from here: port `#527` private-auth removal, then the combined
`#540/#545/#546` dbt and RFC3161 cleanup. The `#528` KEK append production
behavior is already cherry-picked, and the `#532` signer-count enforcement is
implemented with the local CMS-named OneCrypto API; the remaining items remove
larger compatibility surfaces and should come with broader Secure Boot and
BaseCryptLib validation.

## One-line summary for handoff

This board is a public PQC/Crypto Agility proposal set for UEFI validation, certificate, and variable-auth semantics; the most actionable items are the spec PRs around multi-signature rules, CMS semantics, unsupported algorithm rejection, and ECIT capability reporting.
