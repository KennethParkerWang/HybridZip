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
