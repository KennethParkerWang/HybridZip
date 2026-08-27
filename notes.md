# HybridZip R2 Continuation Notes

## GPT Pro Research Handoff - 2026-08-27

- Prepared `docs/research/gpt-pro/` as the handoff location for the next
  research pass. The package does not copy large archives; it indexes the
  authoritative local reports, source files, and tabular evidence.
- Current R2 evidence is the 44-package, 528-row current-hash ledger at
  `results/analysis/r2-complete-ledger/hybridzip-r2-currenthash-cc6d-20260827-r2/`.
  Auto produced 99,720 bytes over 393,216 bytes (2.02880859375 bpb), with zero
  archive-byte gap to the 43-forced-mode oracle. Its aggregate encode/decode
  wall times were 2227.074129/151.5291793 seconds and peak sampled RAM was
  735.3046875 MiB.
- The current local PAQ8px v216 `-1` 32 KiB suite at
  `F:/paq8px/benchmark_paq8px_32KiB_parallel/` is valid evidence for its own
  protocol, but takes a centred slice from each Silesia file. R2 takes the
  leading prefix. The input SHA-256 values differ, so archive-byte and bpb
  comparisons across those two packages are not valid acceptance evidence.
- No Tencent dataset directory, provenance manifest, or completed measurement
  was found under `E:/MIXER`, `E:/MIXER/KU`, or the inspected PAQ8px benchmark
  records. The exact Tencent corpus/version/license/split must be frozen before
  the stated dual-corpus target can be measured.
- Source inspection: `BlockPlanner::plan` computes stored, generic, and
  structure-gated candidate archive sizes before selecting the minimum. The
  existing `StructureActivationRouter` and `HierarchicalActivationRouter` are
  deterministic heuristic gates, not a measured small-candidate production
  shortlist. This explains why Auto reaches the measured oracle but has poor
  encode throughput.

## R2 Research Execution Start - 2026-08-28

- User authorized execution of the external research decision after the
  `baseline-r2-20260828` GitHub tag was pushed.
- The external response correctly identifies same-input PAQ8px baselining and
  a shortlist router as P0, but its proposed zstd integration is already
  represented locally: `third_party/zstd` is vendored, HZ02 exposes mode 2
  (`zstd`), and the CLI already accepts `--zstd-level` and `--block-size`.
  The first fast-policy evidence should therefore measure existing zstd and
  transform paths rather than create a duplicate extension mode.
- Local PAQ8px v216 is available at
  `F:/paq8px/experiment/build/paq8px.exe` and
  `F:/paq8px/PaqBenchStudio/staging-v1.1.0/paq8px.exe`; both are 1,383,936
  bytes with SHA-256
  `F79343702F596A4FA6C7CC3E25F2FA9C05199EAF11F06655B065F687E5F42533`.
- The first implementation artifact is a manifest-driven PAQ8px runner. It
  will default to no-runtime listing and require explicit authorization before
  it creates an experimental package or launches PAQ8px.
- E1 completed without a codec process. The frozen leading-prefix manifest is
  `bench/manifests/silesia-leading-32-64-128.tsv`: 36 canonical rows, 12
  files, 32/64/128 KiB, and SHA-256
  `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D`.
  Both new PowerShell tools passed AST parsing. The PAQ runner `-ListOnly`
  preflight selected `dickens-leading-32k`, reported no runtime start, and
  recorded the expected v216 binary SHA-256.
- E2 same-input smoke passed. The PAQ package is
  `results/experiments/paq8px-v216-level1-silesia-leading-dickens-32k-e2-20260828`.
  It retains 36 canonical CSV rows: 1 `COMPLETE/PASS` and 35 `PENDING` rows,
  so it is a smoke-evidence package and not an importable completed benchmark.
  The PAQ archive is 9,502 bytes (2.319824 bpb), encode/decode each took about
  3.53 seconds with a 372 MiB peak, and all input/decoded bytes matched SHA-256
  `FC42DCB9849222C8704C9DCAE606D075B389B66244FB215035148D6409EC0B31`.
  The pre-existing current-hash HybridZip Auto row has the exact same input
  identity and a 9,598-byte archive (2.343262 bpb). PAQ is smaller by 96 bytes
  on this one case only; no Silesia aggregate conclusion is valid yet.
- The historical, non-comparable PAQ centred-slice level-1 matrix had 36 PASS
  rows, 351.238 encode seconds, and 348.938 decode seconds. It provides only a
  serial full-matrix planning estimate of roughly 12 minutes typical / 30
  minutes conservative for E3; leading-prefix content can change this runtime.

## 2026-08-26 donor audit

- Audited `E:/MIXER/KU/hybridzip-r2` and its provenance inventory.
- All 21 currently registered donor families have an accepted `ported` subset
  under `E:/MIXER/hybridzip/third_party` or `src/r2`.
- OmniZip, MSDZip, Nacrith, L3TC and similar model-oriented material is retained
  as research input but lacks a supplied checkpoint or a self-contained C++17
  decoder closure; it is not labeled as integrated.
- No new download was needed and no existing warehouse material was overwritten.

## Auto routing correction

- `lmic_arithmetic` is now constrained by `family_active[2]` (neural family).
- `delta_binary_packed_zstd` is now constrained by `family_active[4]`
  (numeric family).

## Verification boundary

- Release incremental compilation passed:
  `cmake --build build --config Release --parallel 2`.
- Rebuilt `hybridzip.exe`, `hz_r2_codec_tests.exe`, and affected test targets;
  no test executable was run.
- Per current user constraint, do not run Auto, D40, CTest, batch, or blocks
  larger than 1 KiB without explicit approval.

## Evidence inventory audit

- Read-only scan on 2026-08-26: `results/smoke` contains 74 directories, 50
  directory names containing `1k`, and 59 `verification.json` files.
- The count includes historical router smokes and rebuild duplicates; it does
  not prove that every current mode has a unique final-binary smoke or that
  Auto selection is final.
- No files under `KU` were downloaded or overwritten, and no runtime test was
  launched during this audit.

## Post-build 1 KiB gate

- Evidence directory: `results/smoke/r2-postbuild-1k-20260826`.
- One deterministic random 1 KiB input was forced through mode 41 and mode 42
  using the current Release binary (`FDE6F9...04A75B`). Both decodes were
  byte-exact (`byte_exact=true`).
- Mode 41 archive: 1768 bytes, 13.8125 bpb; encode/decode 0.025065/0.021912 s.
- Mode 42 archive: 1131 bytes, 8.835938 bpb; encode/decode 0.010982/0.012418 s.
- The first invocation failed before compression because a PowerShell argument
  was interpolated incorrectly; no archive was accepted from that invocation.
- No Auto, D40, CTest, batch, retry, or block larger than 1 KiB was run.

## Smoke evidence index

- Added `tools/index_r2_smoke_evidence.ps1`, a metadata-only indexer that scans
  existing `verification.json` files and de-duplicates by mode.
- Run output: `results/analysis/r2-smoke-evidence-index`.
- Filtering to the current Release SHA-256 found 2 qualifying modes (41 and
  42) and 41 missing modes. This is expected because older smoke records use
  older binaries; it is not a runtime failure.
- The indexer performs no encode/decode operation and is suitable as a
  preflight before the eventual final ledger run.
- An unfiltered scan in `results/analysis/r2-smoke-evidence-all` found 14
  qualifying records covering 11 modes (23 and 30..42). The remaining modes
  have no machine-readable mode-tagged 1 KiB record in the current smoke
  inventory, or only router records without an explicit selected-mode field.

## Decoder-visible mode coverage audit

- Static token audit found the same 43 `BlockMode` names (Stored plus modes
  1..42) represented in `block_planner.cpp`, `r2_codec.cpp`, and
  `r2_archive.cpp`.
- No mode was found that is registered only in the planner without a matching
  archive/codec handling branch. This is structural evidence only; it does not
  replace byte-exact runtime evidence or the final archive-byte ledger.

## Donor warehouse validation

- `tools/validate_r2_donors.ps1` passed on 2026-08-26: 2506 checks, covering 21
  donor manifests, 18 port evidence records, 17 Git revisions/origins, one
  release snapshot, three source archives, and 21 license evidence hashes.
- No additional donor with a complete model-free C++17 decoder closure was
  found by this audit; the current candidate set remains the practical R2
  portfolio.

## Current-binary mode 40 gate

- Evidence directory: `results/smoke/r2-postbuild-kanzi-1k-20260826`.
- Forced mode 40 (`kanzi-ans`) encoded and decoded the shared 1 KiB input with
  exact SHA-256 equality. Archive size was 1322 bytes (10.328125 bpb); encode
  and decode wall times were 0.036348 s and 0.024412 s.
- Current-hash smoke index now covers 3/43 modes: 40, 41, and 42.

## Current-binary mode 39 gate

- Evidence directory: `results/smoke/r2-postbuild-lz4-1k-20260826`.
- Forced mode 39 (`lz4`) encoded and decoded the shared 1 KiB input with exact
  SHA-256 equality. Archive size was 1113 bytes (8.695312 bpb); encode and
  decode wall times were 0.031630 s and 0.013472 s.
- Current-hash smoke index now covers 4/43 modes: 39..42.

## Current-binary mode 38 gate

- Evidence directory: `results/smoke/r2-postbuild-wavpack-1k-20260826`.
- Forced mode 38 (`wavpack`) encoded and decoded the shared 1 KiB input with
  exact SHA-256 equality. Archive size was 1289 bytes (10.070312 bpb); encode
  and decode wall times were 0.339487 s and 0.160735 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode38` and covers five
  modes (38..42). Two earlier harness attempts failed before codec launch and
  were not accepted as evidence.

## Current-binary mode 37 gate

- Evidence directory: `results/smoke/r2-postbuild-paq8px-detected-sse-1k-20260826`.
- Forced mode 37 (`paq8px-detected-sse`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1110 bytes (8.671875
  bpb); encode and decode wall times were 0.870678 s and 0.861729 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode37` and covers six
  modes (37..42).

## Current-binary mode 36 gate

- Evidence directory: `results/smoke/r2-postbuild-paq8px-generic-sse-1k-20260826`.
- Forced mode 36 (`paq8px-generic-sse`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1088 bytes (8.5 bpb);
  encode and decode wall times were 0.872760 s and 0.850493 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode36` and covers seven
  modes (36..42).

## Current-binary mode 35 gate

- Evidence directory: `results/smoke/r2-postbuild-paq8px-similarity-sse-1k-20260826`.
- Forced mode 35 (`paq8px-similarity-sse`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1086 bytes (8.484375
  bpb); encode and decode wall times were 0.237022 s and 0.217762 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode35` and covers eight
  modes (35..42).

## Current-binary mode 34 gate

- Evidence directory: `results/smoke/r2-postbuild-paq8px-similarity-1k-20260826`.
- Forced mode 34 (`paq8px-similarity`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1084 bytes (8.46875
  bpb); encode and decode wall times were 0.134416 s and 0.112065 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode34` and covers nine
  modes (34..42).

## Current-binary mode 33 gate

- Evidence directory: `results/smoke/r2-postbuild-paq8px-linear-prediction-1k-20260826`.
- Forced mode 33 (`paq8px-linear-prediction`) encoded and decoded the shared
  1 KiB input with exact SHA-256 equality. Archive size was 1106 bytes
  (8.640625 bpb); encode and decode wall times were 0.109540 s and 0.097196 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode33` and covers ten
  modes (33..42).

## Current-binary mode 32 gate

- Evidence directory: `results/smoke/r2-postbuild-paq8px-record-model-1k-20260826`.
- Forced mode 32 (`paq8px-record-model`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1084 bytes (8.46875
  bpb); encode and decode wall times were 0.122950 s and 0.120585 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode32` and covers eleven
  modes (32..42).

## Current-binary mode 31 gate

- Evidence directory: `results/smoke/r2-postbuild-fastpfor-1k-20260826`.
- Forced mode 31 (`fastpfor`) encoded and decoded the shared 1 KiB input with
  exact SHA-256 equality. Archive size was 1106 bytes (8.640625 bpb); encode
  and decode wall times were 0.118030 s and 0.101729 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode31` and covers twelve
  modes (31..42).

## Current-binary mode 30 gate

- Evidence directory: `results/smoke/r2-postbuild-ctw-1k-20260826`.
- Forced mode 30 (`ctw`) encoded and decoded the shared 1 KiB input with exact
  SHA-256 equality. Archive size was 1135 bytes (8.867188 bpb); encode and
  decode wall times were 0.172544 s and 0.162711 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode30` and covers
  thirteen modes (30..42).

## Current-binary mode 29 gate

- Evidence directory: `results/smoke/r2-postbuild-zpaq-1k-20260826`.
- Forced mode 29 (`zpaq`) encoded and decoded the shared 1 KiB input with exact
  SHA-256 equality. Archive size was 1365 bytes (10.664063 bpb); encode and
  decode wall times were 0.032678 s and 0.013121 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode29` and covers
  fourteen modes (29..42).

## Current-binary mode 28 gate

- Evidence directory: `results/smoke/r2-postbuild-ppmd8-1k-20260826`.
- Forced mode 28 (`ppmd8`) encoded and decoded the shared 1 KiB input with
  exact SHA-256 equality. Archive size was 1194 bytes (9.328125 bpb); encode
  and decode wall times were 0.047398 s and 0.013603 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode28` and covers
  fifteen modes (28..42).

## Current-binary mode 27 gate

- Evidence directory: `results/smoke/r2-postbuild-ppmd7-1k-20260826`.
- Forced mode 27 (`ppmd7`) encoded and decoded the shared 1 KiB input with
  exact SHA-256 equality. Archive size was 1193 bytes (9.320313 bpb); encode
  and decode wall times were 0.030979 s and 0.011583 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode27` and covers
  sixteen modes (27..42).

## Current-binary mode 26 gate

- Evidence directory: `results/smoke/r2-postbuild-jax-compress-portable-1k-20260826`.
- Forced mode 26 (`jax-compress-portable`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1141 bytes (8.914063
  bpb); encode and decode wall times were 0.160004 s and 0.151653 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode26` and covers
  seventeen modes (26..42).

## Current-binary mode 25 gate

- Evidence directory: `results/smoke/r2-postbuild-bgpt-shared-prior-1k-20260826`.
- Forced mode 25 (`bgpt-shared-prior`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1759 bytes (13.742188
  bpb); encode and decode wall times were 0.021639 s and 0.012657 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode25` and covers
  eighteen modes (25..42).

## Current-binary mode 24 gate

- Evidence directory: `results/smoke/r2-postbuild-delta-of-delta-zstd-1k-20260826`.
- Forced mode 24 (`delta-of-delta-zstd`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1099 bytes (8.585938
  bpb); encode and decode wall times were 0.032306 s and 0.012472 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode24` and covers
  nineteen modes (24..42).

## Current-binary mode 23 gate

- Evidence directory: `results/smoke/r2-postbuild-lstm-compress-1k-20260826`.
- Forced mode 23 (`lstm-compress`) encoded and decoded the shared 1 KiB input
  with exact SHA-256 equality. Archive size was 1098 bytes (8.578125 bpb);
  encode and decode wall times were 0.481716 s and 0.507027 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode23` and covers
  twenty modes (23..42).

## Current-binary mode 22 gate

- Evidence directory: `results/smoke/r2-postbuild-shared-neural-lstm-1k-20260826`.
- Forced mode 22 (`shared-neural-lstm`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1099 bytes (8.585938
  bpb); encode and decode wall times were 0.751326 s and 0.730259 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode22` and covers
  twenty-one modes (22..42).

## Current-binary mode 21 gate

- Evidence directory: `results/smoke/r2-postbuild-neural-lstm-1k-20260826`.
- Forced mode 21 (`neural-lstm`) encoded and decoded the shared 1 KiB input
  with exact SHA-256 equality. Archive size was 1094 bytes (8.546875 bpb);
  encode and decode wall times were 0.764807 s and 0.753849 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode21` and covers
  twenty-two modes (21..42).

## Current-binary mode 20 gate

- Evidence directory: `results/smoke/r2-postbuild-cmix-word-zstd-1k-20260826`.
- Forced mode 20 (`cmix-word-zstd`) encoded and decoded the shared 1 KiB input
  with exact SHA-256 equality. Archive size was 1403 bytes (10.960938 bpb);
  encode and decode wall times were 0.037166 s and 0.024788 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode20` and covers
  twenty-three modes (20..42).

## Current-binary mode 19 gate

- Evidence directory: `results/smoke/r2-postbuild-brotli-text-1k-20260826`.
- Forced mode 19 (`brotli-text`) encoded and decoded the shared 1 KiB input
  with exact SHA-256 equality. Archive size was 1088 bytes (8.5 bpb); encode
  and decode wall times were 0.032547 s and 0.010892 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode19` and covers
  twenty-four modes (19..42).

## Current-binary mode 18 gate

- Evidence directory: `results/smoke/r2-postbuild-flac-residual-1k-20260826`.
- Forced mode 18 (`flac-residual`) encoded and decoded the shared 1 KiB input
  with exact SHA-256 equality. Archive size was 1182 bytes (9.234375 bpb);
  encode and decode wall times were 0.037785 s and 0.011712 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode18` and covers
  twenty-five modes (18..42).

## Current-binary mode 17 gate

- Evidence directory: `results/smoke/r2-postbuild-jpegls-1k-20260826`.
- Forced mode 17 (`jpegls`) encoded and decoded the shared 1 KiB input with
  exact SHA-256 equality. Archive size was 1333 bytes (10.414063 bpb); encode
  and decode wall times were 0.031225 s and 0.011573 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode17` and covers
  twenty-six modes (17..42).

## Current-binary mode 16 gate

- Evidence directory: `results/smoke/r2-postbuild-record-transpose-zstd-1k-20260826`.
- Forced mode 16 (`record-transpose-zstd`) encoded and decoded the shared 1 KiB
  input with exact SHA-256 equality. Archive size was 1099 bytes (8.585938
  bpb); encode and decode wall times were 0.044149 s and 0.017307 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode16` and covers
  twenty-seven modes (16..42).

## Current-binary mode 15 gate

- Evidence directory: `results/smoke/r2-postbuild-bcj2-zstd-1k-20260826`.
- Forced mode 15 (`bcj2-zstd`) encoded and decoded the shared 1 KiB input with
  exact SHA-256 equality. Archive size was 1119 bytes (8.742188 bpb); encode
  and decode wall times were 0.033865 s and 0.011954 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode15` and covers
  twenty-eight modes (15..42).

## Current-binary mode 14 gate

- Evidence directory: `results/smoke/r2-postbuild-rans-1k-20260826`.
- Forced mode 14 (`rans`) encoded and decoded the shared 1 KiB input with
  exact SHA-256 equality. Archive size was 1828 bytes (14.28125 bpb); encode
  and decode wall times were 0.038019 s and 0.012341 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode14` and covers
  twenty-nine modes (14..42).

## Current-binary mode 13 gate

- The conventional `r2-postbuild-fastpfor-1k-20260826` directory was already
  occupied by a different historical mode-31 record; no files were overwritten.
- Accepted evidence directory:
  `results/smoke/r2-postbuild-fastpfor-mode13-1k-20260826`.
- Forced mode 13 (`fastpfor`) encoded and decoded the shared 1 KiB input with
  exact SHA-256 equality. Archive size was 1106 bytes (8.640625 bpb); encode
  and decode wall times were 0.031568 s and 0.011354 s.
- The current-hash metadata index is
  `results/analysis/r2-smoke-evidence-index-20260826-mode13` and covers
  thirty modes (13..42).

## Parallel current-Release 1 KiB gate

- The user authorized three-way parallel execution for the remaining forced
  modes. `tools/run_r2_postbuild_parallel_1k.ps1` partitions the work into
  three independent lanes, uses unique output directories, keeps stdout and
  stderr separate, skips accepted records for the current codec hash, and
  enforces a 60-second process timeout.
- Modes 11, 10, 9, 7, 6, 5, 4, 3, 2, 1, and 0 passed byte-exact round-trip on
  the 1024-byte input. Their accepted evidence is under
  `results/smoke/r2-postbuild-*-mode*-1k-20260826-224341-parallel` for the
  first lane run, with mode 2/5 evidence refreshed under the 230041 run.
- Mode 8 (`bwt-rlt-zstd`) did not produce an archive because the random 1 KiB
  BWT block was not reduced by Kanzi RLT. This is an input suitability result,
  not a decoder mismatch; existing 32 KiB D40 evidence remains available. The
  failure is recorded in
  `results/smoke/r2-postbuild-bwt-rlt-zstd-mode8-1k-20260826-230041-parallel/failure.json`.
- The consolidated current-hash index is
  `results/analysis/r2-smoke-evidence-index-20260826-parallel`: 42 unique
  passing modes, missing only mode 8. Auto, D40, CTest, batch, and larger
  inputs were not run.

## 2026-08-27 README alignment audit

- GitHub `origin/main` and the local `main` both resolve to
  `2e7b1b97d56ec90f671141f8afeaecc0e29f4111`; the remote README was the same
  revision as the local README before this documentation update.
- README now states the donor-first rule and links donor/provenance and license
  records. It also links the R2-A through R2-D continuation plan and clarifies
  that phase labels are execution order, not a scope reduction.
- The phrase `five repository tests` was narrowed to five HZ01 baseline tests;
  the current CMake configuration registers 18 CTest targets in total.
- The README status remains intentionally incomplete for R2: 43 decoder-visible
  candidates are exposed, 42/43 have current-Release 1 KiB byte-exact branch
  gates, and the final Auto/archive-byte ledger plus candidate decisions are
  still pending. No runtime experiment was started for this audit.

## 2026-08-27 architecture figure reconciliation

- Updated the hand-authored SVG and PNG exports for the current and full R2
  architecture diagrams. The prior exports incorrectly labeled router layers
  and most portfolio capability as planned and said only 24 paths existed.
- The diagrams now match their Mermaid source: all 43 decoder-visible HZ02
  modes, Layer A/B/C routing, complete archive-byte comparison, per-block
  mode/transform/entropy/CRC32, strict decode, and HZ01 compatibility are
  solid. Only the current-hash Auto plus 43 forced ledger and measured
  retain/retire decision are dashed.
- Verification passed: both SVGs parse as XML, both PNGs render at 1920x1080,
  Mermaid source equals the corresponding fenced Markdown block, and strict
  visual review found no clipping, overlap, or incorrect flow direction. No
  codec process was launched.

## 2026-08-27 Auto test coverage correction

- The R2 Auto test's hand-written non-stored sum ended at mode 39, leaving the
  current mode 40 (Kanzi ANS), mode 41 (LMIC), and mode 42
  (delta-binary-packed Zstd) outside its assertion.
- Replaced the range with `std::accumulate(begin + 1, end, 0U)`, so the test
  remains correct for every current non-stored block mode and future additions
  that expand `blocks_by_mode`.
- `hz_r2_codec_tests` rebuilt and linked successfully. It was deliberately not
  run, so this is compilation evidence rather than a fresh CTest result.

## 2026-08-27 Auto archive-byte accounting

- Auto selection compares payload plus transform metadata. The 16-byte block
  header and 4-byte CRC are invariant across candidates, so their omission did
  not change selection; it did make exported selected/oracle telemetry smaller
  than the corresponding archive by 20 bytes per block plus the 40-byte HZ02
  archive header.
- `BlockPlanner` now adds the per-block fixed overhead and `compress_file`
  seeds aggregate selected/oracle totals with the archive header. The Auto test
  now asserts selected bytes equal archive bytes and oracle bytes do not exceed
  selected bytes for both compressible and stored cases.
- The rebuilt Release hash is
  `F650AE7E662FDC28F82CF18F4279F7BAAA4433A9C3890EAAC94970A73D11432B`.
  No codec execution followed the rebuild. The FDE6 current-hash smoke records
  are therefore preserved as prior-build evidence pending the final ledger.

## 2026-08-27 final-ledger preflight after rebuild

- `tools/run_r2_complete_ledger.ps1 -ListOnly` accepted the rebuilt Release
  path and reported 44 packages, 43 forced modes, 12 Silesia files, and the
  32 KiB scope with `runtime_started=false`.
- The runner did not create an experiment package or launch a codec process.

## 2026-08-27 forced-mode archive-byte telemetry repair

- The first minimal forced `stored` gate on the Auto-accounting Release found
  that its CLI `selected` and `oracle` fields retained only the 40-byte HZ02
  archive header. This did not affect the archive bytes or the final ledger,
  which reads files directly, but it made forced-mode CLI measurements
  misleading.
- `r2_codec.cpp` now fills forced-mode decision telemetry from the final
  serialized block size: 16-byte block header, 4-byte CRC32, transform
  metadata, and payload. The statistics therefore use the same full-file
  meaning in Auto and forced operation.
- Release compilation passed for `hybridzip` and `hz_r2_codec_tests`. A new
  random 1 KiB forced-stored smoke wrote a 1,084-byte archive, reported
  `selected=1084` and `oracle=1084`, and decoded with equal input/output
  SHA-256. Evidence is
  `results/smoke/r2-telemetry-stored-1k-20260827-v2/verification.json`.
- The active Release SHA-256 after this repair is
  `CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`.
  The earlier `F650...1432` build remains a distinct pre-repair hash; it must
  not be combined with either the historical branch evidence or a future
  current-hash ledger.

## 2026-08-27 forced-mode ledger attribution gate

- The complete-ledger derivation previously trusted a forced package's name
  when exporting its selected mode. Its `block_types` field was present but
  was not used to prove that the requested forced mode was actually serialized.
- `derive_r2_complete_ledger.ps1` now parses the CLI block count record,
  rejects missing, malformed, unknown, duplicate, zero-count, and
  wrong-total records, and requires every forced block to equal its requested
  mode. It also requires the fixed 64 KiB ledger block-size parameter.
- In-memory PowerShell checks passed for `zstd=1`, Auto's canonical
  `stored=1;zstd=1` mode order, a forced-mode substitution, and a zero-count
  malformed record. No archive was created and no codec process was started.

## 2026-08-27 immediate forced-mode runner gate

- The Silesia runner now applies the same HZ02 `block_types` validation right
  after an R2 encode succeeds and before it starts decode. This makes an
  unexpected forced-mode fallback a failed case at its source instead of a
  late final-ledger failure after the remaining packages have run.
- The gate verifies a known decoder-visible mode, positive non-duplicate
  counts, the expected 64 KiB block count, and exact requested-mode identity
  for forced runs. Auto remains free to record any valid candidate combination.
- The PowerShell parser plus in-memory `zstd=1`, Auto two-block,
  forced-substitution, and wrong-block-count checks passed. No archive was
  created and no codec process was started.

## 2026-08-27 segment-oracle forced attribution gate

- `run_r2_segment_oracle.ps1` now validates the actual HZ02 `block_types`
  record after each successful encode and before it enters a forced-mode oracle
  comparison. A fallback or malformed archive therefore cannot become a local
  oracle winner under the requested donor name.
- Its parser plus `zstd=1`, Auto two-block, forced-substitution, and
  wrong-block-count checks passed in memory. The runner remains non-overwriting
  and requires explicit runtime authorization.
# Current-hash ledger result (2026-08-27)

The authorized ledger `hybridzip-r2-currenthash-cc6d-20260827-r2` is complete.
It contains Auto plus 43 forced HZ02 modes, 44 packages, 12 Silesia 32 KiB
prefix cases per package, and 528 validated rows. Every row is COMPLETE/PASS,
has zero codec exit codes, uses Release SHA-256
`CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`, and has
byte-exact input/archive/decoded SHA-256 evidence.

Auto totals 99,720 complete archive bytes over 393,216 input bytes (2.028809
bpb). The forced archive-byte oracle is identical on all 12 cases: aggregate
and per-case Auto gap are zero. Auto selects `paq8px-detected-sse` five times
and `paq8px-generic-sse` seven times; they are the only observed forced-mode
oracle winners. Other donor paths remain in the product and are not deleted;
their no-win status is limited to this one prefix matrix.

Derived files, strict analysis, figures, and the round-review report are under
`results/analysis/r2-complete-ledger/hybridzip-r2-currenthash-cc6d-20260827-r2/`.
The separate segment-oracle runtime experiment remains unrun by design.

## 2026-08-28 R2 experiment design and execution boundary

- The uploaded R2 decision is now the active implementation specification. Tier
  A uses the frozen 12 Silesia files at leading 32/64/128 KiB; Tier B is
  complete Silesia plus an owner-approved Tencent/OASum corpus.
- Ratio evidence uses complete archive bytes, identical input SHA-256 values,
  byte-exact decoded SHA-256 values, and explicit block size/codec hashes.
  `bpb = 8 * archive_bytes / input_bytes`; shortlist regret is measured against
  the full 43-mode oracle, not against a single forced donor.
- E3 (36 serial PAQ8px cases) remains pending. This turn does not launch E3,
  OASum download, D40, Auto sweeps, CTest, or larger runtime matrices.
- E4 starts with an encoder-only `auto-k8` policy. It preserves all 43 HZ02
  decoder IDs and the existing full `auto` policy. K=8 is a measured
  shortlist, not a product claim: promote it only if E5 held-out recall and
  regret gates pass.
- K=8 mandatory modes are Stored, Zstd, Paq8pxGenericSse, and
  Paq8pxDetectedSse. The four additional slots are selected from text
  (Ppmd7/Ppmd8/BrotliText/BwtZstd), x86
  (Fse/Lzma/X86BcjZstd/Bcj2Zstd), numeric
  (Fse/Lzma/DeltaZstd/ShuffleZstd), or generic
  (Fse/Lzma/Ppmd7/Ppmd8).

## 2026-08-28 E4 auto-k8 implementation result

- Added `BlockFeaturesV1` and `mode_ranker` under `src/r2/routing/`. The
  extractor uses integer per-mille statistics and four deterministic classes;
  the ranker always retains Stored, Zstd, Paq8pxGenericSse, and
  Paq8pxDetectedSse, then adds four class-specific modes.
- Added `CandidatePolicy::AutoK8` and CLI spelling `--r2-mode=auto-k8`.
  The policy is encoder-only; HZ02 IDs 0..42 and HZ01 decoding are unchanged.
- Focused tests passed: `hz_structure_routing_tests`; Release builds passed for
  `hybridzip`, `hz_structure_routing_tests`, and `hz_r2_codec_tests`.
- The latest 1 KiB smoke passed with eight candidates, a 463-byte archive, mode
  37 (`paq8px-detected-sse`) selected, and equal input/decoded SHA-256. Full
  codec tests and corpus matrices were intentionally not run.
- Evidence: `results/smoke/r2-auto-k8-1k-20260828-v3/verification.json`.
- The current executable hash is
  `C3831DA767B75F06039C52BEA936D8F4DF633E8CB383DE6ECF11D5E8953A9D31` and
  represents the current uncommitted working tree, not the baseline tag.

## 2026-08-28 E6 fast-policy implementation

- Added `CandidatePolicy::Fast` and CLI spelling `--r2-mode=fast`. It executes
  the existing zstd backend at level 3 (or lower if explicitly requested),
  serializes existing HZ02 mode 2, and keeps forced-policy archive telemetry.
- The fast policy is separate from full Auto and K=8 ratio routing. It is an
  encoder policy for the 0.16-0.20 MB/s CPU target; its ratio must not be mixed
  with the PAQ8px ratio claim.
- The focused codec test now covers a 1 KiB Fast round trip. The required
  32/64/128 KiB throughput and P50/P95 latency measurements are still pending.

The focused Fast smoke passed with 1,024 input bytes, a 662-byte mode-2 HZ02
archive, and byte-exact decode. Evidence is
`results/smoke/r2-fast-1k-20260828/verification.json`; the rebuilt executable
hash is `7B8388DB81FCA3994BCE112B7AA712B224CBBCF4C034DDA3765505113334C4FE`.

The guarded `tools/run_silesia_experiment.ps1` now supports `auto-k8` and
`fast`, passes `--block-size` for 32/64/128 KiB experiments, clamps Fast to
zstd level 3, and checks Fast's actual serialized mode as `zstd`. Fast/32 KiB
and Auto-K8/128 KiB `-ListOnly` probes passed; no codec process was started.

## 2026-08-28 target execution order

- E5 is the next ratio-router proof: a held-out K=2/K=4/K=8 archive ledger
  against full Auto, with tie-aware winner recall and complete-archive regret.
  The current offline 32 KiB derivation is only a preview: 12/12 recalled
  winners and 0-byte regret on the existing matrix, not held-out evidence.
- E6 is the next fast-path proof: three warm timing repeats at 32, 64, and
  128 KiB, reporting byte-exact decode, encode/decode MB/s, P50/P95 latency,
  RAM, and complete archive bytes. The CPU target is >=0.16 MB/s in both
  directions for every block size.
- E3 is held separate: all 36 frozen Silesia prefixes must be processed by
  PAQ8px v216 `-1` before an aggregate same-input ratio comparison exists.
- OASum remains unmaterialized pending approval of its 1.065 GB test artifact
  and CC-BY-SA-3.0 treatment; no Tencent coverage has been claimed.

## 2026-08-28 E5 ablation and matrix tooling

- Added encoder-only `auto-k2` and `auto-k4` policies. K=2 is Stored plus
  generic PAQ8px SSE; K=4 adds zstd and detected PAQ8px SSE; K=8 remains the
  class-conditioned policy. These are experiment cardinalities, not archive
  modes and not promotion claims.
- Added `candidate_modes=<mode-id>:<block-count>` and `full_oracle=0|1` to
  HZ02 CLI statistics. Both are encoder telemetry only and are excluded from
  HZ02 archive bytes.
- The first new 1 KiB package failed before archive publication because its
  telemetry assertion treated the planner's backend-order candidate array as
  a BlockMode-ID array. The explicit mapping repair did not alter archive or
  decoder code. The retained v2 smoke passed K=2/K=4/K=8 with candidate counts
  2/4/8, identical 463-byte archives, and byte-exact SHA-256 reconstruction.
  Evidence: `results/smoke/r2-shortlist-ablation-1k-20260828-v2/`.
- Added `tools/run_r2_e5_e6_matrix.ps1`. Its AST parse and default list-only
  plans passed. E5 plans 12 child packages and E6 plans 12; each full-scope
  matrix would make 864 codec invocations. Runtime requires both
  `-ListOnly:$false` and `-AuthorizeRuntimeExperiment` and is still unrun.
- Current-build 1 KiB compatibility evidence also passed for Fast and HZ01:
  Fast uses mode 2/zstd and a 662-byte HZ02 archive; HZ01 uses a 537-byte
  archive. Both decoded SHA-256 values equal the shared input. The initial
  recorder wrote the Fast record then stopped on HZ01's intentionally empty
  encode stdout; the retained archive hashes are documented in
  `results/smoke/r2-e5-e6-compat-1k-20260828-v1/verification-recovery.json`.
- `docs/research/R2_IMPLEMENTATION_AUDIT_20260828.md` now provides a
  requirement-to-evidence matrix. It makes no PAQ-ratio, CPU-throughput,
  Tencent, GPU, or K=8-promotion claim until the corresponding runtime gates
  are completed.

## 2026-08-28 attachment-driven execution record

### Source read

- User-provided decision text:
  `C:/Users/Administrator/.codex/attachments/b96760cb-802c-4f54-b886-1fce9454f953/pasted-text.txt`
  (1,053 lines) was read in full.
- Its fixed direction is one HZ02 archive contract with `ENC_RATIO_V1`,
  `ENC_FAST_V1`, and oracle-only full portfolio evaluation. It explicitly
  rejects treating a PAQ-heavy ratio policy as evidence of the CPU target.

### Current implementation versus decision

- Implemented now: one HZ02 container; HZ01 compatibility; 43 existing HZ02
  modes; full Auto; encoder-only K=2/K=4/K=8 policies; Fast policy using
  existing zstd mode 2; guarded E3/E5/E6 runners.
- Not yet implemented: the specified 28-feature fixed-point ranker and frozen
  2,644-byte model; `MODE_FAST_EXT_V1`; transform selection within Fast;
  independent-block executor; CPU reference `LZ_RANS_V1`; GPU path.
- Current zstd provenance is version 1.6.0 under `third_party/zstd`; it is
  not the decision's requested v1.5.7 pin. Keep measured Fast data labelled
  with the current executable hash and do not call it v1.5.7 acceptance.

### Measurement decisions

- E3 and E6 are allowed to run on the frozen Tier-A Silesia inputs. E3 uses
  PAQ8px v216 `-1` on all 36 leading prefixes. E6 uses one warmup plus three
  retained repeats for the existing Fast policy at every 32/64/128 KiB
  input/block-size pair.
- E5 is intentionally sequenced after E3/E6. Its 864 encode/decode
  invocations include full Auto and therefore repeatedly materialize PAQ
  candidates; co-running it would confound both timing and machine load.
- OASum remains unmaterialized. The decision's 1.065 GB test artifact and
  CC-BY-SA-3.0 redistribution boundary require a separate owner approval.

### E6 result

- Completed package:
  `results/experiments/hybridzip-r2-e6-fast-full-20260828-retry1/`.
- Current executable SHA-256:
  `E65526F9DFF3F93844E004D63C7B2A4E4F219B5EAB3F1B3D3ABCF0B301F65003`.
- 432/432 rows are `COMPLETE/PASS`: 108 warmups and 324 retained timing
  measurements. All nine input-scope/internal-block-size cells meet the
  0.16 MB/s CPU floor; minimum encode/decode rates are 0.6977/0.6476 MB/s.
- This is a mode-2 zstd 1.6.0 Fast baseline. It does not establish the
  attachment's Fast K=4, extension-mode, transform, block-executor, or GPU
  requirements.

### E3 result

- Completed package:
  `results/experiments/paq8px-v216-level1-silesia-leading-e3-20260828/`.
- 36/36 rows are COMPLETE/PASS. All manifest prefix/input/decoded identities
  match; all codec exit codes are zero. E3 exactly reproduced the E2 Dickens
  32 KiB archive (9,502 bytes and its SHA-256).
- Aggregate PAQ complete archive bytes are 97,555 / 182,710 / 342,298 at
  32/64/128 KiB input scopes, or 1.984762 / 1.858622 / 1.741018 bpb.
- Across all 36 inputs, PAQ produced 622,563 archive bytes from 2,752,512
  input bytes (1.809440 bpb). Its total encode/decode time was 323.025 /
  323.464 seconds and maximum sampled RAM was 1,147.49 MiB.
- The historical current-hash HybridZip 32 KiB Auto ledger is 99,720 bytes
  on the same 393,216 input bytes. PAQ's current same-input total is 97,555,
  a 2,165-byte (2.1711%) advantage. This is a ratio gap, not a claim about
  the new working-tree executable or 64/128 KiB HybridZip until E5 runs.

### Matrix recovery

- `tools/run_r2_e5_e6_matrix.ps1` now accepts `-Resume`. It validates the
  experiment ID, stage, codec path/hash, dataset path, files, scopes, block
  sizes, policies, and repeats before it resumes any child package.
- A complete package is read-only validated and returned without invoking a
  codec. The completed E6 package passed this check while its
  `matrix_rows.csv` SHA-256 remained
  `671E3C42E8C678FB2D05E94030C6CA626AEC6BD7848AF76765DD0D3D86462D0C`.

## 2026-08-28: Attachment-Driven Protocol Decision

- Input: `C:/Users/Administrator/.codex/attachments/b96760cb-802c-4f54-b886-1fce9454f953/pasted-text.txt`.
- Applied direction: retain one HZ02 container, HZ01 compatibility, and IDs
  0..42; append an extensible fast mode as ID 43 rather than replacing the
  existing portfolio.
- Evidence tracks remain separate: `ENC_RATIO_V1` uses identical inputs and
  complete archive bytes versus PAQ8px; `ENC_FAST_V1` uses the CPU throughput
  contract. One track does not satisfy the other.
- Existing evidence: E3 PAQ8px has 36/36 byte-exact rows at 1.809440 bpb
  across frozen Tier-A prefixes; current Fast has 432/432 byte-exact rows and
  clears the 0.16 MB/s CPU floor in all nine cells.
- Deferred: E5 shortlist promotion, final ratio result, Tencent/OASum
  coverage, and GPU performance are unproven.
- Donor boundary: `E:/MIXER/KU/zstd-v1.5.7/` has verified provenance, while
  production still builds the existing vendored zstd 1.6.0.

## 2026-08-28: MODE_FAST_EXT_V1 Initial Gate

- Appended HZ02 `BlockMode::FastExtension = 43`; IDs 0..42 remain unchanged.
- Initial metadata is exactly `01 00 00 00`: extension version 1, standard
  zstd codec 0, transform none 0, and uLEB128 side-information length 0.
- The zstd frame is built with zstd checksum, content size, and dictionary ID
  disabled because HZ02 already carries CRC32 and raw size; zstd workers stay
  at zero.
- Release `hybridzip` and `hz_r2_codec_tests` built and linked. The full test
  executable was not run.
- `results/smoke/r2-fast-extension-1k-20260828/verification.json` records a
  deterministic 1 KiB gate: HZ02 ID 43 decoded byte-exactly, external
  `D:/anaconda/Library/bin/zstd.exe` decoded the extracted frame byte-exactly,
  a version-2 mutation was rejected with no output, and HZ01 also decoded
  byte-exactly.

## 2026-08-28: Fast K=4 runtime gate

- The current Release build evaluates four Fast candidates: stored, raw
  Mode-43 zstd extension, best transformed Mode-43 zstd extension, and LZ4.
- A deterministic 1 KiB 32-bit counter input produced `candidates=4` and
  selected Mode 43. The extension selected `transform_id=2` (bitshuffle) with
  side-information width `2`.
- Complete archive accounting: 159 bytes total, 94-byte zstd payload, 9-byte
  HZ02 metadata field (4-byte CRC plus 5-byte extension metadata).
- HybridZip decode was byte-exact: input SHA-256
  `CCA5E448EAF942DC406AF4BC778128571B7B8B21228BE8581E7E37FBA97B0211`.
  Archive SHA-256 is
  `DCB2974A494AFADD9372F7E810A7C28D41ABFC8C5F0C439DCB03CAB34B7B89CE`.
- External `D:/anaconda/Library/bin/zstd.exe` decoded the extracted frame with
  exit code 0 and recovered 1,024 transformed bytes. Its transformed-byte
  SHA-256 is `816CCD294309C281D0BD6D1772284E08FE28F6F26F1ABB6B7FE64A33D27D7924`.
- Evidence is retained under
  `results/smoke/r2-fast-k4-counter-1k-20260828-v1/verification.json`.
- This is a correctness/dispatch gate only. The previous 432-row E6 package
  remains the mode-2 Fast baseline; no post-change corpus throughput claim is
  made until a new E6 matrix is run.

## 2026-08-28: Fast matrix preflight repair

- `tools/run_r2_e5_e6_matrix.ps1` already listed `fast-ext` in its internal
  44-mode registry, but `tools/run_silesia_experiment.ps1` omitted it from
  the public `ValidateSet`. The child runner would reject a forced Mode-43
  experiment before starting a codec process. The parameter value is now
  registered in both places.
- Both PowerShell scripts passed AST parsing. A no-runtime Fast E6 preflight
  for `dickens`, 32 KiB input and 32 KiB internal blocks planned four child
  packages (warmup plus three retained repeats), eight codec invocations, and
  reported `runtime_started=false`.
- `docs/R2_FORMAT.md` now documents the Mode-43 table row, descriptor bytes,
  IDs, side-information constraints, zstd frame flags, and CRC/inverse order.

## 2026-08-28: F1 28-feature fixed-point bootstrap

- `BlockFeaturesV1` now has all 28 attachment-defined integer features:
  entropy, byte classes, run/delta/periodicity, bounded sampled LZ, plausible
  x86 targets, and packed UTF-8/magic/OOD flags. No runtime float is used.
- `FixedPointRankerModelV1` has the specified 2,644-byte layout for modes
  0..42, version `0x00010000`, signed-64 score accumulation, low-ID ties,
  and a canonical CRC32 validation. Mode 43 remains Fast-only.
- The weights are hand-set bootstrap values, explicitly not a trained model.
  No-leakage fitting requires the forced-mode E5 matrix and file-level splits.
- `hz_structure_routing_tests.exe` passed after exercising model CRC/version,
  repeatable feature vectors, text/x86/numeric/generic gates, and compressed
  magic flags.
- `results/smoke/r2-f1-fixed-ranker-auto-k8-1k-20260828-v1/` records a
  current-Release 1 KiB byte-exact `auto-k8` gate: 8 candidates
  (`0,2,3,4,27,28,36,37`), stored selected, archive 1,084 bytes.
