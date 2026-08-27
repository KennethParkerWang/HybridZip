# HybridZip R2 Continuation Plan

## Goal

Continue the donor-first R2-A through R2-D implementation until the current
portfolio is a runnable, byte-exact HybridZip product with HZ01 compatibility,
decoder-visible routing, and a final archive-byte experiment ledger.

## GPT Pro Research Handoff (2026-08-27)

Goal: prepare a reproducible, evidence-bounded research packet for improving
HybridZip toward the user-specified PAQ8PX-1 ratio and CPU/GPU throughput
targets without changing the current codec during this preparation task.

- [x] Audit the current R2 ledger, local PAQ8px baseline records, router
  implementation, dataset availability, and result-import contract.
- [x] Create a concise research brief, source-material index, and copy-ready
  GPT Pro prompt under `docs/research/gpt-pro/`.
- [x] Record the non-comparable-input boundary between the existing R2 leading
  prefix ledger and the PAQ8px centred-slice benchmark.

Status: research packet complete. No codec implementation or runtime
experiment was started. The next implementation decision awaits the external
research result and a fixed Tencent dataset identity/hardware target.

## R2 Evidence-Gated Execution (2026-08-28)

### Objective

Turn the external research decision into measured R2 work without weakening
HZ01 compatibility or confusing a router proposal with a demonstrated ratio or
throughput result.

### Targets

- R0 - reproducibility: one frozen leading-prefix Silesia manifest covering
  12 files x 32/64/128 KiB, with source and prefix SHA-256 identities.
- R1 - fair ratio baseline: PAQ8px v216 `-1` and HybridZip consume exactly the
  same manifest inputs. Complete archive bytes and byte-exact decoded hashes
  are required before any comparison is made.
- R2 - ratio router: K=8 candidate shortlist retains stored plus both observed
  PAQ8px-SSE winners; it is compared with the 43-mode oracle on held-out
  inputs using tie-aware winner recall and complete-archive regret.
- R3 - CPU fast policy: use the existing zstd and reversible fast paths first,
  then measure 32/64/128 KiB throughput and latency against the 0.16 MB/s CPU
  floor. A fast result is not treated as a PAQ-ratio result.
- R4 - Tencent/OASum acceptance: deferred until the owner explicitly approves
  the 1.065 GB `test.jsonl` download and CC-BY-SA-3.0 handling.
- R5 - GPU: deferred until a CPU reference fast path passes byte-exact gates or
  is shown to miss the CPU target.

### Execution Phases

- [x] E0: Tag and push the pre-research implementation baseline as
  `baseline-r2-20260828`.
- [x] E1: Generate and validate the Silesia leading-prefix input manifest; add
  a guarded PAQ8px runner that consumes only that manifest. The frozen manifest
  is `bench/manifests/silesia-leading-32-64-128.tsv` (36 rows; SHA-256
  `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D`).
- [x] E2: Execute one 32 KiB same-input HybridZip/PAQ8px smoke pair, then
  inspect hashes and complete archive accounting before expanding coverage.
  `dickens-leading-32k` used input SHA-256
  `FC42DCB9849222C8704C9DCAE606D075B389B66244FB215035148D6409EC0B31`.
  PAQ8px `-1` produced a 9,502-byte archive (2.319824 bpb) with a byte-exact
  round-trip; the existing same-input current-hash HybridZip Auto archive is
  9,598 bytes (2.343262 bpb). This is one smoke case only, not a corpus claim.
- [ ] E3: Complete the authorized 36-case same-input Silesia baseline only
  after E2 passes and its expected runtime is reported. Historical centred
  `-1` runs took 700.176 codec seconds for 36 cases; this supports a 30-minute
  leading-prefix runtime reservation but is not a ratio comparison.
- [x] E4: Implement deterministic `BlockFeaturesV1` and a rule-only K=8
  shortlist while retaining current full Auto as the oracle path.
- [ ] E5: Measure K=2/K=4/K=8 recall, regret, candidate count, latency, and
  memory on frozen inputs; only promote a shortlist that passes the gates.
- [ ] E6: Establish `ENC_FAST_V1` from existing zstd/transform paths and
  evaluate block-size and throughput targets.
- [ ] E7: Stage OASum provenance and data only after explicit data/license
  authorization; run the separate complete-corpus acceptance tier afterward.
- [ ] E8: Consider block executor and GPU `LZ_RANS_V1` only after measured E6
  results justify them.

### Decisions

- The external report proposed a new zstd donor/extension mode, but the local
  source already vendors zstd and exposes raw/transform zstd HZ02 modes. E6
  therefore begins with the current paths and `--zstd-level`, not a duplicate
  donor import or a new archive mode.
- No runtime corpus sweep is implicitly authorized by this plan. Each runner
  defaults to a no-runtime listing and requires an explicit authorization
  switch.
- OASum is a prospective Tencent text/records corpus, not a downloaded or
  accepted product corpus yet.

### Status

E1 and E2 are complete. The manifest/runner infrastructure passed PowerShell
AST checks and a no-runtime `-ListOnly` preflight. E2 ran only PAQ8px encode
and decode because the current-hash HybridZip Auto ledger already contains the
same frozen 32 KiB bytes. E3 remains intentionally pending rather than
starting a 36-case serial PAQ8px run without a dedicated authorization.

## 2026-08-28 implementation checkpoint

- [x] Freeze the external R2 experiment design and acceptance gates in
  `docs/research/R2_EXPERIMENT_DESIGN_20260828.md`.
- [x] E4 implementation: add encoder-only `auto-k8` policy with deterministic
  integer byte features and a fixed eight-mode shortlist. HZ02 decoder-visible
  mode IDs remain unchanged.
- [x] E4 verification: build `hybridzip`, `hz_structure_routing_tests`, and
  `hz_r2_codec_tests`; run only focused unit/smoke checks, including one 1 KiB
  `auto-k8` round trip. No corpus sweep is authorized by this checkpoint.

### E4 acceptance gates

1. `auto` behavior and HZ01 decode remain unchanged.
2. `auto-k8` exposes exactly eight requested modes for a nonempty block:
   Stored, Zstd, both observed PAQ8px SSE modes, and four class-specific modes.
3. The shortlist is deterministic and integer-feature based.
4. A 1 KiB `auto-k8` archive decodes byte-for-byte.
5. Full-43 recall/regret is not claimed until E5 is run on held-out inputs.

E4 completed on 2026-08-28. `hz_structure_routing_tests` passed; the affected
Release targets compiled and linked. The latest focused `auto-k8` smoke passed
with a 1,024-byte input, 463-byte archive, eight materialized candidates, mode
37 as winner, and byte-exact decode. Evidence is in
`results/smoke/r2-auto-k8-1k-20260828-v3/verification.json`. The full
`hz_r2_codec_tests` binary was rebuilt but not executed to respect the minimal
runtime boundary.

E6 implementation started on 2026-08-28: the encoder exposes `--r2-mode=fast`
and reuses HZ02 zstd mode 2 with a level-3 cap. Its 1 KiB correctness gate is
covered in `hz_r2_codec_tests` and the target has been rebuilt; the 32/64/128
KiB throughput ledger remains pending.

The 1 KiB Fast correctness gate also passed: 662-byte archive, mode 2
(`zstd`), and byte-exact SHA-256 decode. Evidence is in
`results/smoke/r2-fast-1k-20260828/verification.json`. No throughput claim is
made from this small smoke.

The guarded Silesia runner now accepts `auto-k8` and `fast`, records the
selected internal block size, and validates 32/64/128 KiB block-count and
forced-mode attribution. `-ListOnly` probes for Fast/32 KiB and Auto-K8/128
KiB passed without starting a codec process.

E5 ablation implementation completed on 2026-08-28. The encoder now accepts
`auto-k2` and `auto-k4` alongside `auto-k8`, and stdout records
`candidate_modes` plus the full-Auto indicator. The shared 1 KiB smoke passed
with exactly 2/4/8 materialized modes, 463-byte archives, and byte-exact
decode in `results/smoke/r2-shortlist-ablation-1k-20260828-v2/`.

The first ablation smoke package (`...-v1`) was retained after a new telemetry
assertion exposed an incorrect assumption: the planner's candidate-byte array
uses backend construction order rather than BlockMode-ID order. The mapping is
now explicit in `block_planner.cpp`; the non-overwriting v2 package passed.
No archive-format or decoder behavior was changed by this repair.

`tools/run_r2_e5_e6_matrix.ps1` now provides a guarded, non-overwriting E5/E6
orchestrator. PowerShell AST and both full-scope `-ListOnly` plans passed with
`runtime_started=false`; no corpus codec process was launched.

- [x] Audit the current working tree against the evidence-gated objective in
  `docs/research/R2_IMPLEMENTATION_AUDIT_20260828.md`. It confirms the exact
  implementation boundary and keeps E3/E5/E6 acceptance claims pending.

The current executable also passed the focused Fast and HZ01 compatibility
smokes on the shared 1 KiB input. Fast retained mode 2 and a 662-byte archive;
HZ01 retained a 537-byte archive; both decoded hashes equal the input. The
first recorder stopped only because HZ01 intentionally emits no encode stdout.
`verification-recovery.json` records the retained artifact hashes without a
second codec run.

### Next execution checkpoint

- [ ] Commit and push the R2 K=8/Fast implementation milestone with its
  source, frozen manifest, experiment design, and small metadata-only evidence.
- [ ] E5: run the non-overwriting held-out K=2/K=4/K=8 ledger, comparing each
  shortlist with the full Auto archive-byte oracle. Promotion requires at
  least 99.5% tie-aware winner recall and at most 0.02% aggregate regret.
- [ ] E6: run three retained Fast-policy timing repeats at each frozen block
  size after warmup; require byte-exact decode, report P50/P95 and peak RAM,
  and test the 0.16 MB/s CPU floor independently for encode and decode.
- [ ] E3: complete the independent 36-case same-input PAQ8px v216 `-1`
  baseline before calculating any aggregate HybridZip/PAQ ratio comparison.
- [ ] E7: obtain explicit owner approval before materializing OASum or making
  a Tencent-dataset coverage claim.

## Phases

- [x] Phase 1: Record current state and preserve the existing R2 plan.
- [x] Phase 2: Audit the next R2-D donor candidates in `E:/MIXER/KU`.
- [x] Phase 3: Port one license-cleared, runnable candidate into HZ02.
- [x] Phase 4: Release-build and run the allowed 1 KiB branch round-trip gates
  (42/43 current-hash modes pass; mode 8 is unsuitable for the random input).
- [x] Phase 5: Update provenance, format, ledger, and choose the next candidate.
- [x] Phase 6: Regenerate the complete current-hash Auto/archive-byte ledger
  and retire or retain candidates after explicit experiment authorization.

### Phase 6 execution checklist

- [x] Add a resumable runner manifest for Auto plus all 43 decoder-visible R2
  modes (44 packages: Auto plus 43 forced paths).
- [x] Add a read-only ledger derivation step that validates complete archive
  bytes, timing, peak memory, byte-exact hashes, and actual forced block-mode
  attribution before writing outputs.
- [x] Add a separately guarded intra-file segment-oracle runner. It writes
  offset/length provenance plus archive bytes, bpb, time, RAM, byte-exact
  hashes, and a per-segment forced-mode winner without changing HZ02 bytes.
- [x] Run the authorized current-Release corpus experiment and preserve one
  package per mode without overwriting prior evidence.
- [x] Compare Auto/oracle gaps and retain or retire candidates from measured
  results.
- [x] Update the product status and final technical report with the ledger.
- [x] Run one current-Release HZ01 compatibility smoke and record exact hashes.

## Constraints

- Preserve HZ01 and PROFILE_V1 behavior.
- Prefer existing donor code; do not rewrite a mature codec from scratch.
- Before final implementation is declared complete, gate each new branch with
  one deterministic 1 KiB encode/decode smoke only.
- Do not rerun Auto, D40, CTest, batch, or larger blocks without explicit user
  approval.
- Keep downloaded material and provenance under `E:/MIXER/KU`.

## Status

Phase 2 audit found no additional model-free donor with a complete runnable
C++17 closure. The current warehouse donors are already represented as
`ported` or explicitly limited by unavailable checkpoints. The existing
portfolio is therefore the candidate set for the next accounting pass. A
small Auto family-gating defect was fixed in `src/r2/block/block_planner.cpp`;
mode 41 is now gated by the neural family and mode 42 by the numeric family.
Release incremental compilation passed on 2026-08-26. The branch-level Phase 4
gate is complete under the 1 KiB-only policy; the final Auto/CTest ledger is
separate; it was intentionally deferred until the complete ledger was
authorized.

Read-only evidence audit on 2026-08-26 found 50 smoke directories whose names
contain `1k` and 59 `verification.json` files under `results/smoke`. These
include historical and rebuild duplicates, so they are evidence inventory only
and are not counted as a 43-mode final ledger. No new runtime test was started.

Post-build gate update: the current Release binary completed forced 1 KiB
encode/decode smokes for mode 41 (`lmic-arithmetic`) and mode 42
(`delta-binary-packed-zstd`). Both produced 1024 decoded bytes with exact
input/output SHA-256 equality. Phase 4 remains open for the final portfolio
because these checks are branch gates, not Auto or corpus evidence.

Added and ran `tools/index_r2_smoke_evidence.ps1`; after filtering to the
current Release hash, it reports unique 1 KiB evidence for 2/43 modes. The
remaining records are historical-hash evidence and require refreshed gates or
the final authorized ledger run.

An unfiltered metadata scan found 14 qualifying records for 11/43 explicit
mode tags (23 and 30..42). Router-only records are intentionally not inferred
as selected-mode evidence. This is now the preflight baseline for any future
one-mode-at-a-time smoke authorization.

Donor warehouse validation passed with 2506 checks across 21 manifests and
their provenance/license evidence. No additional complete model-free C++17
decoder closure was found, so implementation proceeds with the existing
portfolio while the final ledger remained pending at that checkpoint.

Tooling checkpoint on 2026-08-27: `tools/run_r2_complete_ledger.ps1` now emits
a fixed, resumable 44-package manifest (Auto plus 43 forced decoder-visible
modes) and requires `-AuthorizeRuntimeExperiment`
before starting any codec process. `tools/derive_r2_complete_ledger.ps1` is a
read-only derivation step: it refuses incomplete packages, mixed codec hashes,
duplicate cases, mismatched archive lengths, or non-byte-exact SHA-256
round-trips. `-ListOnly` was parsed and run successfully; no runtime experiment
was started. Phase 6 runtime execution was later authorized and completed
under ledger ID `hybridzip-r2-currenthash-cc6d-20260827-r2`.

Documentation checkpoint on 2026-08-27: `README.md` now separates the completed
HZ01 baseline measurements from the active HZ02 R2 implementation, records the
42/43 current-Release branch-gate boundary, and shows both compatibility and R2
paths. `docs/PRODUCT_STATUS.md` now uses the current Release SHA-256; the final
R2 ledger and candidate decisions remain intentionally unclaimed. Its HZ01
baseline table retains the historical `2D28...` hash that is embedded in the
completed product and Silesia evidence packages.

README alignment checkpoint on 2026-08-27: the public README now explicitly
records donor-first provenance links and the R2-A through R2-D continuation
plan. It distinguishes five HZ01 baseline tests from the 18 CTest targets
registered by the current CMake configuration. This is documentation-only
progress; no codec process or runtime experiment was started.

Ledger validation checkpoint on 2026-08-27: `derive_r2_complete_ledger.ps1`
now rejects missing/non-finite/negative timing or memory values, non-zero codec
exit codes, malformed codec hashes, and inconsistent peak-memory aggregates
before deriving any R2 result table. PowerShell parsing passed; no package was
executed.

Canonical mode-order checkpoint on 2026-08-27: the Silesia runner, family
runner, Silesia validator, and family analyzer now use the decoder-visible
`BlockMode` order `0..42`. The analyzer accepts historical prefix logs and
current 43-mode logs while rejecting mismatched or non-canonical block labels.
All four scripts passed PowerShell parser checks; the Silesia runner accepted a
mode-24 `-ListOnly` probe, the complete-ledger runner reported 44 packages with
`runtime_started=false`, and `git diff --check` passed. No codec process or
runtime experiment was started.

The next one-mode gate completed for mode 40 (`kanzi-ans`) on the current
Release binary. The current-hash evidence index now covers 3/43 modes (40..42)
with byte-exact 1 KiB records.

Execution policy update on 2026-08-26: the user authorized three-way parallel
execution for the remaining current-Release 1 KiB forced-mode gates. Each mode
must retain a unique output directory and separate stdout/stderr; index and
documentation refresh remain serialized after the workers finish.

Mode 39 (`lz4`) then passed the same one-mode 1 KiB gate. Current-hash
coverage is now 4/43 modes (39..42).

Mode 38 (`wavpack`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1289 bytes (10.070312 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
5/43 modes (38..42). Two earlier harness attempts failed before codec launch
and produced no accepted codec evidence.

Mode 37 (`paq8px-detected-sse`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1110 bytes (8.671875 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 6/43 modes (37..42).

Mode 36 (`paq8px-generic-sse`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1088 bytes (8.5 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 7/43 modes (36..42).

Mode 35 (`paq8px-similarity-sse`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1086 bytes (8.484375 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 8/43 modes (35..42).

Mode 34 (`paq8px-similarity`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1084 bytes (8.46875 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 9/43 modes (34..42).

Mode 33 (`paq8px-linear-prediction`) then passed the same one-mode 1 KiB gate
on the current Release binary. The archive was 1106 bytes (8.640625 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 10/43 modes (33..42).

Mode 32 (`paq8px-record-model`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1084 bytes (8.46875 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 11/43 modes (32..42).

Mode 31 (`fastpfor`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1106 bytes (8.640625 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
12/43 modes (31..42).

Mode 30 (`ctw`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1135 bytes (8.867188 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
13/43 modes (30..42).

Mode 29 (`zpaq`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1365 bytes (10.664063 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
14/43 modes (29..42). Evidence is in
`results/smoke/r2-postbuild-zpaq-1k-20260826/verification.json`; the refreshed
index is `results/analysis/r2-smoke-evidence-index-20260826-mode29`.

Mode 28 (`ppmd8`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1194 bytes (9.328125 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
15/43 modes (28..42). Evidence is in
`results/smoke/r2-postbuild-ppmd8-1k-20260826/verification.json`; the refreshed
index is `results/analysis/r2-smoke-evidence-index-20260826-mode28`.

Mode 27 (`ppmd7`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1193 bytes (9.320313 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
16/43 modes (27..42). Evidence is in
`results/smoke/r2-postbuild-ppmd7-1k-20260826/verification.json`; the refreshed
index is `results/analysis/r2-smoke-evidence-index-20260826-mode27`.

Mode 26 (`jax-compress-portable`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1141 bytes (8.914063 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 17/43 modes (26..42). Evidence is in
`results/smoke/r2-postbuild-jax-compress-portable-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode26`.

Mode 25 (`bgpt-shared-prior`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1759 bytes (13.742188 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 18/43 modes (25..42). Evidence is in
`results/smoke/r2-postbuild-bgpt-shared-prior-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode25`.

Mode 24 (`delta-of-delta-zstd`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1099 bytes (8.585938 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 19/43 modes (24..42). Evidence is in
`results/smoke/r2-postbuild-delta-of-delta-zstd-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode24`.

Mode 23 (`lstm-compress`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1098 bytes (8.578125 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 20/43 modes (23..42). Evidence is in
`results/smoke/r2-postbuild-lstm-compress-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode23`.

Mode 22 (`shared-neural-lstm`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1099 bytes (8.585938 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 21/43 modes (22..42). Evidence is in
`results/smoke/r2-postbuild-shared-neural-lstm-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode22`.

Mode 21 (`neural-lstm`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1094 bytes (8.546875 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
22/43 modes (21..42). Evidence is in
`results/smoke/r2-postbuild-neural-lstm-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode21`.

Mode 20 (`cmix-word-zstd`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1403 bytes (10.960938 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 23/43 modes (20..42). Evidence is in
`results/smoke/r2-postbuild-cmix-word-zstd-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode20`.

Mode 19 (`brotli-text`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1088 bytes (8.5 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 24/43 modes (19..42). Evidence is in
`results/smoke/r2-postbuild-brotli-text-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode19`.

Mode 18 (`flac-residual`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1182 bytes (9.234375 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 25/43 modes (18..42). Evidence is in
`results/smoke/r2-postbuild-flac-residual-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode18`.

Mode 17 (`jpegls`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1333 bytes (10.414063 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
26/43 modes (17..42). Evidence is in
`results/smoke/r2-postbuild-jpegls-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode17`.

Mode 16 (`record-transpose-zstd`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1099 bytes (8.585938 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 27/43 modes (16..42). Evidence is in
`results/smoke/r2-postbuild-record-transpose-zstd-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode16`.

Mode 15 (`bcj2-zstd`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1119 bytes (8.742188 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
28/43 modes (15..42). Evidence is in
`results/smoke/r2-postbuild-bcj2-zstd-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode15`.

Mode 14 (`rans`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1828 bytes (14.28125 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
29/43 modes (14..42). Evidence is in
`results/smoke/r2-postbuild-rans-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode14`.

Mode 13 (`fastpfor`) then passed the same one-mode 1 KiB gate on the current
Release binary. The original output name was already occupied by a different
historical mode-31 record, so the accepted evidence uses a unique mode-tagged
directory. The archive was 1106 bytes (8.640625 bpb), decoded to 1024 bytes,
and matched the input SHA-256 exactly. Current-hash coverage is now 30/43
modes (13..42). Evidence is in
`results/smoke/r2-postbuild-fastpfor-mode13-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode13`.

The user then authorized three-way parallel execution for the remaining
current-Release 1 KiB gates. The reusable worker is
`tools/run_r2_postbuild_parallel_1k.ps1`; it uses unique per-mode directories,
separate stdout/stderr, a 60-second per-process timeout, and skips accepted
current-hash evidence. Modes 11, 10, 9, 7, 6, 5, 4, 3, 2, 1, and 0 passed
byte-exact encode/decode. Mode 8 (`bwt-rlt-zstd`) was intentionally recorded as
not applicable to the random 1 KiB gate because Kanzi RLT did not reduce the
BWT block; its failure record is
`results/smoke/r2-postbuild-bwt-rlt-zstd-mode8-1k-20260826-230041-parallel/failure.json`.
The current-hash index is
`results/analysis/r2-smoke-evidence-index-20260826-parallel` and reports
42/43 unique passing modes with only mode 8 missing. No Auto, D40, CTest, or
larger-block test was run.

The next implementation checkpoint is metadata-only: use the mode registry
emitted by `tools/index_r2_smoke_evidence.ps1` to keep the 43-mode evidence
boundary auditable. The final Auto archive-byte ledger is now complete under
the current Release hash; only the separately guarded segment-oracle runtime
experiment remains unrun.

Tooling checkpoint on 2026-08-27: the Silesia runner, family runner, and
package validator now accept every decoder-visible R2 mode through mode 42,
including the PAQ8px, WavPack, Kanzi ANS, LMIC, and delta-binary paths. This
changes no archive bytes and does not count as runtime evidence; the Phase 6
ledger was completed separately.

Metadata checkpoint completed on 2026-08-26 without starting the codec. The
current Release hash index is
`results/analysis/r2-smoke-evidence-index-20260826-registry`; its fixed
43-row registry contains 42 `PASS` modes and mode 8 as
`MISSING_CURRENT_HASH_EVIDENCE`. The index and the registry changes were
committed and pushed as `47766f1`.

Pre-ledger instrumentation checkpoint on 2026-08-27: added
`tools/run_r2_segment_oracle.ps1` for the planned local-heterogeneity oracle
experiment. It is non-overwriting and refuses to launch the codec without
`-AuthorizeRuntimeExperiment`; `-ListOnly` reports the exact segment/mode
matrix. It retains failed rows, classifies the documented non-reducible
`bwt-rlt-zstd` condition as inapplicable, and writes `manifest.csv`,
`results.csv`, and per-segment `oracle.csv`. It is a research-specific package,
not an Experiment Ledger Silesia-prefix import package.

Parser checkpoint: the first AST check identified PowerShell's variable-plus-
colon interpolation rule in one error message. The message was changed to use
a formatting expression for the literal regex end-anchor, and the conditional
return was expanded for compatibility. The final PowerShell AST parse passed;
the 1 KiB one-segment `-ListOnly` preflight reported 43 forced modes plus
optional Auto (88 maximum codec invocations) with `runtime_started=false`.
The script registry also matched the canonical decoder-visible 43-mode order.

Documentation checkpoint on 2026-08-27: reconciled both hand-authored 16:9
architecture SVG/PNG exports with the corrected Mermaid sources and current
R2 implementation. Both figures now depict all 43 decoder-visible modes, the
integrated Layer A/B/C Auto route, archive-byte comparison, HZ02 CRC/strict
decode, and HZ01 compatibility as source-integrated. The current-hash
44-package ledger and its retain/retire decision are now recorded in the
derived ledger and round-review report. XML parsing,
1920x1080 PNG rendering, Mermaid/Markdown identity checks, and visual review
passed; no codec process was started.

Auto regression checkpoint on 2026-08-27: corrected
`tests/r2_codec_tests.cpp` so its compressible-block Auto assertion sums every
non-stored decoder-visible mode (`1..42`) rather than a stale handwritten
range ending at mode 39. The affected test target rebuilt and linked in Release
configuration; it was not executed under the current no-CTest constraint.

Archive-accounting checkpoint on 2026-08-27: Auto telemetry now adds the
fixed 16-byte block header, 4-byte CRC, and 40-byte archive header so selected
and oracle byte totals can be compared directly with `archive_bytes`.
`hybridzip` and `hz_r2_codec_tests` rebuilt and linked without running tests.
The rebuilt Release SHA-256 is
`F650AE7E662FDC28F82CF18F4279F7BAAA4433A9C3890EAAC94970A73D11432B`.
The prior `FDE6...A75B` 42/43 smoke index remains historical behavior evidence,
not current-hash evidence; the final authorized ledger must use the new hash.

Final-ledger preflight on 2026-08-27: the rebuilt Release passed
`tools/run_r2_complete_ledger.ps1 -ListOnly`, reporting 44 packages (Auto plus
43 forced modes), all 12 Silesia files, 32 KiB scope, and
`runtime_started=false`. No output package or codec process was created.

Forced-mode accounting repair on 2026-08-27: the initial random 1 KiB forced
stored gate showed that CLI `selected` and `oracle` totals included only the
40-byte HZ02 archive header. This did not change archive bytes or the final
file-based ledger, but it made single-mode diagnostics inconsistent with Auto.
`src/r2/codec/r2_codec.cpp` now supplies the final serialized block bytes for
every forced policy, while Auto keeps its portfolio/oracle accounting. Both
affected Release targets compiled and linked. The replacement forced-stored
1 KiB gate produced a 1,084-byte archive, printed `selected=1084` and
`oracle=1084`, and had exact input/output SHA-256 equality; evidence is
`results/smoke/r2-telemetry-stored-1k-20260827-v2/verification.json`. The
active Release SHA-256 is now
`CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`.
This is a targeted diagnostic and round-trip gate only. It is not Auto,
corpus, D40, CTest, or a final ledger run.

Forced-mode attribution gate on 2026-08-27: the final derivation now parses
each row's CLI `block_types` record and refuses unknown, duplicate, zero,
wrong-total, or forced-mode-mismatched blocks. A forced donor can therefore
only receive an archive-byte result if the archive actually records that donor
for every forced block. The fixed 64 KiB ledger block-size parameter is also
validated before rows enter the Auto/oracle comparison. PowerShell parsing and
in-memory positive/negative parser checks passed without creating an archive
or launching the codec.

Immediate forced-mode gate on 2026-08-27: `run_silesia_experiment.ps1` now
performs the same valid-mode, count, and requested-mode validation immediately
after each R2 encode and before decode. Incorrect forced attribution therefore
stops that case rather than consuming the rest of a 44-package ledger. Parser
and in-memory positive/negative checks passed without a codec process.

Segment-oracle attribution gate on 2026-08-27:
`tools/run_r2_segment_oracle.ps1` now applies the same forced block-mode proof
before any archive can enter its per-segment oracle. Its parser and in-memory
positive/negative checks passed without creating a runtime package or starting
the codec.

Ledger recovery checkpoint on 2026-08-27: the authorized current-hash runner
revalidated 43 existing packages (516 rows) and completed the four missing
32 KiB cases for `paq8px-detected-sse`. A stale `sao.hz2.tmp` left by the
interrupted prior run exposed a runner error: empty telemetry stdout caused a
regex null-input exception. `run_silesia_experiment.ps1` now removes only the
current case's known archive/temp/decoded paths before a rerun and treats empty
telemetry as `UNKNOWN`; PowerShell AST parsing passed. All 44 packages now
need final manifest status reconciliation and read-only derivation.

Current-Release compatibility checkpoint on 2026-08-27: a deterministic 1 KiB
HZ01 encode/decode smoke passed with a 537-byte archive, 1,024 decoded bytes,
and exact input/decoded SHA-256 equality. Evidence is
`results/smoke/r2-final-hz01-1k-20260827/verification.json`; it uses the same
Release hash as the R2 ledger.
