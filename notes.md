# HybridZip R2 Continuation Notes

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
