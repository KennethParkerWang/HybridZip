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

## 2026-08-28 F3 Fast Block Executor

### Objective

Implement the attachment's independent-block execution boundary for
`ENC_FAST_V1` without changing any HZ02 bytes, mode IDs, or non-Fast encoder
policy. Archive serialization must remain in canonical input block order even
when compression decisions finish in another order.

### Work items

- [x] F3.0: freeze the small-scope experiment design in
  `docs/research/R2_F3_BLOCK_EXECUTOR_EXPERIMENT_DESIGN_20260828.md`.
- [x] F3.1: add a bounded Fast-only worker executor and `--threads` option.
- [x] F3.2: run the permitted 1 KiB / four-block lossless and deterministic
  archive gate at one and two workers.
- [x] F3.3: commit and push the implementation/evidence milestone as
  `501759c` (`feat(r2): add Fast block executor`).
- [x] F3.4a: add `-FastThreadCount` to the guarded E6 matrix and thread-count
  resume/telemetry checks; AST and one-file `-ListOnly` preflights passed for
  one and two workers without starting a codec process.
- [ ] F3.4b: only after explicit authorization, run the separate post-change
  E6 Fast K=4 32/64/128 KiB timing matrix.

### Decision

Only `CandidatePolicy::Fast` may use worker threads in this checkpoint.
`auto` owns mutable family telemetry and stays serial; the K=2/K=4/K=8
policies and every forced mode also retain their existing serial behavior.

### Status

**F3.1-F3.4a complete.** F3.4b requires explicit authorization for a
post-change Fast K=4 timing matrix. No corpus benchmark is running or
authorized by this work item.

## 2026-08-28 attachment-driven target execution

### Decision

The uploaded R2 decision is the execution reference for this phase. It is
applied to the checked-in HZ02 implementation, not treated as permission to
rewrite the existing container or renumber its decoder-visible modes.

- [x] Commit and push the E4/E5/E6 tooling milestone as
  `439e948 feat(r2): add shortlist ablations and E5/E6 matrix`.
- [x] Freeze the implementation objectives, experiment scopes, acceptance
  metrics, and current/report gap analysis in
  `docs/research/R2_TARGETS_AND_EXECUTION_20260828.md`.
- [x] E3: produce the same-input PAQ8px v216 `-1` rows for the 36-row frozen
  Silesia manifest. All rows are COMPLETE/PASS, their manifest input and
  decoded SHA-256 values match, and output is preserved in
  `results/experiments/paq8px-v216-level1-silesia-leading-e3-20260828/`.
  The PAQ total is 622,563 bytes / 2,752,512 input bytes (1.809440 bpb).
- [x] E6: measure the current Fast policy with one warmup and three retained
  repeats at all frozen 32/64/128 KiB input and block-size combinations.
  All 432 rows passed byte-exactly; the 324 retained rows pass the >= 0.16
  MB/s encode/decode floor in all nine cells. Evidence:
  `results/experiments/hybridzip-r2-e6-fast-full-20260828-retry1/` and
  `docs/research/R2_E6_FAST_RESULTS_20260828.md`.
- [ ] E5: after E3/E6 packages are reviewed, run the full-Auto versus
  K=2/K=4/K=8 regret matrix as a separate long-running job. It is deliberately
  not co-scheduled with E3 because full Auto materializes PAQ candidates.
  The current historical Auto rate gives a 12.99-hour encode-only lower bound
  for the three-block-size full-Auto rows, excluding shortlist encodes and all
  decodes.
- [x] Make E5/E6 recovery safe: the parent matrix runner resumes only a
  package with matching stage, codec SHA-256, dataset, matrix dimensions, and
  policies. Completed packages are checked without launching the codec.
- [ ] F1: expand the rule-only feature extractor into the attachment's frozen
  28-feature, integer-only ranker after its no-leakage label source exists.
- [ ] F2: add the one-ID `MODE_FAST_EXT_V1` only after a pinned zstd source
  and independent standard-frame vectors are available.
- [ ] F3: add canonical-order independent-block execution only after the
  single-thread Fast baseline is measured.
- [ ] F4: do not begin GPU `LZ_RANS_V1` before F2/F3 and measured Fast results.

### Constraints and recorded gaps

- HZ01 and HZ02 modes `0..42` remain compatibility gates.
- Existing `third_party/zstd` is version 1.6.0. The attachment names 1.5.7;
  current Fast results are therefore an accurately identified baseline, not
  donor-version acceptance evidence. The 2.43 MiB BSD-3-Clause 1.5.7 source
  has not been downloaded or imported.
- OASum is blocked pending an owner decision on the 1.065 GB `test.jsonl` and
  CC-BY-SA-3.0 treatment. No Tencent coverage is claimed.
- A K=8 promotion requires a forced-mode, tie-aware oracle in addition to
  the current E5 full-Auto reference. Current E5 tooling must state that
  limitation in every resulting report.

### Errors encountered

- The first E6 launch used nested `powershell -File` and passed
  `-ListOnly:$false` as a string. Parameter binding rejected it before the
  script created an output directory or launched the codec. The retry uses
  direct invocation from the current PowerShell session so the switch remains
  a Boolean value.
- The direct E6 retry completed its first 36-row child package, then the
  parent matrix runner read an undefined `$LASTEXITCODE` after invoking a
  PowerShell child script. The failed parent package is preserved. The runner
  now checks `$?`, and the full matrix will be restarted under a new ID rather
  than overwriting partial evidence.

### Status

E3 and E6 have been authorized for the frozen Tier-A runtime. E6 ran first on
an otherwise idle CPU and passed its current-build baseline gate. E3 also
completed successfully. Their output directories are unique and
non-overwriting. E5 remains queued as the next PAQ-heavy workload; it must not
be started concurrently with another timing experiment. Its resume protocol
was verified against the completed E6 package without altering the recorded
matrix hash.

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

## Attachment-Driven R2 Execution (2026-08-28)

### Goal

Implement the attachment's additive donor-first R2 direction while preserving
the HZ02 decoder contract, HZ01 decoding, and every existing HZ02 ID 0..42.
Keep compression-ratio and throughput claims on separate evidence tracks.

### Active Plan

- [x] P0: Publish the evidence snapshot `31fc6e6` to `origin/main`.
- [x] P0: Freeze Tier-A Silesia inputs and complete E3 PAQ8px and E6
  current-Fast baselines.
- [x] P0: Write `docs/research/R2_EXPERIMENT_PROTOCOL_20260828.md`.
- [x] P0: Add `MODE_FAST_EXT_V1` as HZ02 ID 43 with a standard zstd-frame,
  no-transform initial payload and strict decoder metadata validation.
- [x] P0: Build and execute focused mode-43/HZ01 1 KiB byte-exact gates.
  Independent `zstd.exe` decoding and malformed-version rejection passed; E5
  was not started.
- [x] P1: Add shuffle, bitshuffle, delta/XOR, and BCJ Mode-43 choices and
  form the Fast K=4 policy. The 1 KiB gate selected bitshuffle and round-trip
  decoding passed; corpus-level E6 evidence remains pending.
- [x] P1: Add the 28-feature integer extractor and 2,644-byte fixed-point
  bootstrap ranker with CRC validation and hard family gates. It is not
  trained from forced-mode labels; K=8 remains experimental until E5 supplies
  no-leakage labels and held-out regret evidence.
- [ ] P1: Run E5 in a dedicated CPU window. Its known lower-bound cost is
  about 13 hours for full Auto and it needs forced-oracle data for promotion.
- [x] P1: Add canonical-order Fast-only block parallelism and pass its 1 KiB
  deterministic archive gate. Post-change Fast timing remains pending.
- [ ] P2: Start GPU `LZ_RANS_V1` only after Fast extension/block-executor
  evidence exists.

### Current Status

**Fast K=4 implementation and its 1 KiB gate are complete.** The verified
zstd v1.5.7 donor is staged in `E:/MIXER/KU`, while production presently uses
vendored zstd 1.6.0. The current Mode-43 smoke reports the latter and is not a
v1.5.7 acceptance result.

## 2026-08-28 Fast K=4 checkpoint

- [x] Implement the append-only `MODE_FAST_EXT_V1` path at HZ02 mode 43.
- [x] Add stored, raw extension, transformed extension, and LZ4 candidates to
  the Fast policy; cap extension zstd level at 3.
- [x] Run one deterministic 1 KiB Fast K=4 smoke. It evaluated exactly four
  candidates, selected mode 43 with bitshuffle width 2, produced a 159-byte
  archive, and passed HybridZip byte-exact decode.
- [x] Independently decode the extracted Mode-43 zstd payload with the local
  zstd executable; 1,024 transformed bytes were recovered.
- [x] Register `fast-ext` in the guarded child Silesia runner's parameter
  validation. Both runner scripts passed PowerShell AST parsing, and the
  Fast E6 one-file/one-cell `-ListOnly` preflight planned four child packages
  with `runtime_started=false`.

### Next execution target

- [x] Commit and push this Mode-43/Fast K=4 milestone as `01d129e`.
- [ ] Rerun the guarded E6 matrix under a new non-overwriting ID after the
  policy change; do not reuse the prior mode-2 baseline package.
- [ ] Keep E5 full-Auto/K=2/K=4/K=8 regret work as a separate PAQ-heavy job;
  no long run is started by this checkpoint.
- [ ] Continue with no-leakage model fitting, post-change Fast timing, pinned
  zstd 1.5.7 production choice, and then GPU LZ-RANS only when their
  acceptance gates are justified by measured results.

## 2026-08-28 F1 Model Identity Telemetry

- [x] Canonically serialize the 2,644-byte ranker model in little-endian
  field order and derive a SHA-256 through the existing libzpaq donor adapter.
- [x] Emit ranker version, CRC32, and SHA-256 in every R2 encoder telemetry
  line; persist them in E5/E6 rows and reject a completed package with mixed
  identities.
- [x] Pin the V1 model identity in the routing unit test and run one allowed
  1 KiB `auto-k8` byte-exact smoke.

### Result

The frozen bootstrap identity is version `0x00010000`, CRC32 `0x1025B343`,
and SHA-256 `4B1AC26C40AD4DA50312FD3B694D7E636FB768C2336FE773BC82D36424C27A4B`.
The current smoke archive is 1,084 bytes for a 1,024-byte random input and
has matching input/decoded SHA-256. Evidence:
`results/smoke/r2-f1-model-identity-1k-20260828-v2/verification.json`.
No corpus matrix was launched.

### Error record

The first input-generator attempt used a PowerShell unsigned-integer cast that
reported overflow as non-terminating errors. It launched no codec process and
is excluded; the retained v2 evidence uses the system random-number source.

## 2026-08-28 Forced-Mode Oracle Labels

- [x] Extend the complete R2 ledger runner with an explicit recorded internal
  block size so 32 KiB forced results can be comparable to K=8 blocks.
- [x] Add a no-write/list-only-aware forced-oracle derivation that retains all
  tied minimum complete-archive modes and joins a matching E5 package to
  calculate tie-aware K2/K4/K8 recall.
- [x] Add and pass a 43-mode synthetic self-test: a tied `zstd,fse` oracle is
  detected, K2 misses it, and K4/K8 hit it.
- [ ] Run the dedicated 12-file 32 KiB forced ledger after runtime approval;
  it plans 1,056 codec invocations and must remain separate from E5/E6.

The detailed protocol is
`docs/research/R2_FORCED_ORACLE_EXPERIMENT_DESIGN_20260828.md`. No corpus
codec process was launched for this checkpoint.

## 2026-08-28 E5 Forced-Oracle Evidence Binding

- [x] Add the optional E5-only `-ForcedOracleLedgerPath` identity to the
  guarded matrix runner. A non-empty path is normalized, persisted in
  `experiment.json`, and required to match during resume.
- [x] Validate the completed ledger read-only before the PAQ-heavy E5 matrix,
  then derive and retain tie-aware results in `<e5-package>\forced-oracle`
  before marking an attached E5 package complete.
- [x] Verify PowerShell AST parsing, the no-codec synthetic tied-winner test,
  and a one-file E5 `-ListOnly` plan (four child packages; eight planned codec
  invocations; `runtime_started=false`).
- [ ] Run the separate 12-file forced ledger, then invoke the E5 matrix with
  that ledger path in a dedicated runtime window. This remains 1,056 forced
  invocations plus the separate E5 workload; do not co-schedule it with Fast
  timing.

### Status

The attachment's tie-aware router measurement now has a recoverable evidence
path in the runner. No corpus encode, decode, or PAQ-heavy E5 job was started
by this implementation checkpoint. The implementation and documentation are
committed and pushed as `996d59d`.

## 2026-08-28 zstd v1.5.7 Donor Provenance Reconciliation

- [x] Verify the existing `E:/MIXER/KU/zstd-v1.5.7/` release artifact rather
  than downloading it again: archive bytes/SHA-256, annotated tag, peeled
  commit, extracted source version, selected BSD-3-Clause license hash, and
  deterministic path/content tree identity all match the recorded donor.
- [x] Add `docs/provenance/zstd-v1.5.7.json` and correct the target document's
  stale "not downloaded" statement.
- [ ] Decide and implement a distinct production source replacement only if
  v1.5.7 is required over the current vendored 1.6.0. That change requires
  independent frame vectors, HZ01/HZ02 decode gates, and new Fast evidence;
  staging provenance alone is not an encoder acceptance result.

## 2026-08-28 K=8 No-Leakage Training Data Interface

- [x] Add `hz_r2_feature_dump`, a read-only build target linked to the exact
  C++ `BlockFeaturesV1` and fixed-point ranker. It emits all 28 features,
  classification, K=8 modes, and model identity without archive construction.
- [x] Add `export_r2_ranker_training_set.ps1`, which joins a complete 32 KiB
  forced-oracle package to source prefixes after SHA-256 validation and enforces
  an explicit file-level training/validation split.
- [x] Build the exporter and pass its two-file synthetic test: 28 features per
  row, one training file, one validation file, zero codec invocations, and no
  archive encode/decode.
- [x] Rebuild and run `hz_structure_routing_tests.exe` after adding the
  interface; it passed without constructing an archive.
- [ ] Run the completed forced ledger through the exporter, fit a frozen model
  without validation leakage, then measure held-out E5 recall/regret before
  changing K=8 from experimental.

## 2026-08-28 Attachment Latency Telemetry Gate

### Objective

Close the only identified E6 evidence gap: retain exact Fast block
queue-plus-service and service-only timing samples so P50/P95 latency claims
are computed from blocks rather than from file-level wall-clock proxies.

- [x] L1: inspect the Fast executor, CLI telemetry, child runner, and matrix
  aggregation against the attachment's required fields.
- [x] L2: add encoder-only timing samples and preserve them through E6 matrix
  rows and exact quantile summaries; HZ02 bytes and all non-Fast paths stay
  unchanged.
- [x] L3: build the focused targets and run one deterministic 1 KiB/four-block
  Fast smoke at one and two workers, checking byte-exact decode, identical
  archive bytes, and nonempty latency telemetry.
- [x] L4: commit and push the telemetry checkpoint as `f94dc29`.
  Post-change E6 remains a
  separately authorized corpus runtime experiment.
- [x] L5: make the E6 telemetry parser recompute P50/P95 from raw samples and
  reject mismatched percentiles, unequal paired counts, or a queue-plus-service
  sample below its service-only counterpart.

### Status

**Latency telemetry checkpoint complete.** Release targets and PowerShell AST parsing passed. The
1 KiB four-block gate passed at one and two workers with byte-exact decode,
identical 540-byte archives, and four paired latency samples per run. Evidence:
`results/smoke/r2-fast-latency-telemetry-1k-20260828-v1/verification.json`.
The existing E5 forced oracle remains a 1,056-codec-call job and will not be
started by this telemetry checkpoint. The next execution target is the
separately authorized forced-mode ledger, followed by real ranker fitting and
the post-change E6 matrix.

## 2026-08-28 Benchmark Environment Identity Gate

- [x] H1: add one structured, non-overwriting environment capture tool for
  CPU/RAM, GPU/driver when discoverable, power plan, compiler, codec hash, and
  Git source identity.
- [x] H2: require a matching environment fingerprint for E5/E6 matrix resume
  and forced-oracle ledger resume; persist `environment.json` in every new
  runtime package.
- [x] H3: replace the child runner's placeholder source revision with the
  actual Git commit plus a dirty-tree marker, and reject incompatible resume.

### Status

**Environment identity gate complete.** PowerShell AST validation passed for
all four affected runners. Two captures produced fingerprint
`D2361A6DBEA69EC701710515FB46651EE4A5CBE5F5EC2ACEE20F84E99E87607D` on the
current host; the codec SHA-256, active power plan, compiler version, CPU, and
GPU entries were present. An attempted overwrite was rejected. No codec process
was launched by this gate.
