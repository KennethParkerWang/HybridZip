# HybridZip R2 Continuation Notes

## Runtime checkpoint 2026-08-29 08:11 +08:00

- E5 remains `302/432`, all durable rows `PASS`, failures `0`.
- PID `6844` continues the `ooffice` 128 KiB Auto encode and remains
  responsive; no summary or duplicate runner exists.

## Runtime checkpoint 2026-08-29 08:11 +08:00

- `ooffice` 64 KiB completed byte-exactly, bringing E5 to `302/432` `PASS`
  rows.
- The same runner is now processing `ooffice` 128 KiB in `auto-b128-r1`.

## Runtime checkpoint 2026-08-29 08:08 +08:00

- E5 remains `301/432`, all durable rows `PASS`, failures `0`.
- Codec PID `17452` continues `ooffice` 64 KiB Auto encoding and remains
  responsive; no summary or duplicate runner exists.

## Runtime checkpoint 2026-08-29 08:05 +08:00

- E5 remains `301/432`, all durable rows `PASS`, failures `0`.
- PID `17452` continues `ooffice` 64 KiB Auto encoding in `auto-b128-r1`;
  preserve the existing runner and package.

## Runtime checkpoint 2026-08-29 08:02 +08:00

- `nci` 128 KiB and `ooffice` 32 KiB passed byte-exactly, bringing E5 to
  `301/432` `PASS` rows.
- The same runner is now processing `ooffice` 64 KiB in `auto-b128-r1`.

## Runtime checkpoint 2026-08-29 08:00 +08:00

- `nci` 128 KiB completed byte-exactly, bringing E5 to `300/432` `PASS` rows.
- The same runner is now processing `ooffice` 32 KiB in `auto-b128-r1`.

## Runtime checkpoint 2026-08-29 07:57 +08:00

- E5 remains `299/432`, all durable rows `PASS`, failures `0`.
- PID `30012` continues the `nci` 128 KiB Auto encode since 07:44; it is
  responsive and CPU-active. Preserve the protected temporary archive.

## Runtime checkpoint 2026-08-29 07:55 +08:00

- E5 remains `299/432`, all recorded rows `PASS`, failures `0`.
- The single-thread `nci` 128 KiB Auto encode is active; the protected `.tmp`
  archive is still zero bytes, as expected before atomic rename.

## Runtime checkpoint 2026-08-29 07:53 +08:00

- E5 remains `299/432`, all durable rows `PASS`, failures `0`.
- Codec PID `30012` continues the `nci` 128 KiB Auto encode since 07:44;
  `summary.json` is absent and the runner has not been restarted.

## Runtime checkpoint 2026-08-29 07:52 +08:00

- E5 is `299/432`, all recorded rows `COMPLETE/PASS`, failures `0`.
- PID `30012` continues the `nci` 128 KiB Auto encode in `auto-b128-r1`;
  `summary.json` remains absent and no duplicate runner exists.

## Runtime checkpoint 2026-08-29 07:50 +08:00

- E5 has `299/432` durable `COMPLETE/PASS` rows; `auto-b128-r1` is `11/36`.
- Codec PID `30012` is active on `nci` 128 KiB; no summary or duplicate runner
  exists.

## Runtime checkpoint 2026-08-29 07:45 +08:00

- `nci` 64 KiB completed byte-exactly, bringing E5 to `299/432` `PASS` rows.
- The same runner is now processing `nci` 128 KiB in `auto-b128-r1`.

## Runtime checkpoint 2026-08-29 07:39 +08:00

- `nci` 32 KiB completed byte-exactly; E5 remains at `298/432` while `nci`
  64 KiB is encoding.
- Codec PID `7112` and parent PID `30912` are the only active experiment
  processes; no failure or duplicate run was observed.

## Runtime checkpoint 2026-08-29 07:37 +08:00

- `nci` 32 KiB completed byte-exactly, bringing E5 to `298/432` `PASS` rows.
- The same runner is now processing `nci` 64 KiB in `auto-b128-r1`.

## Runtime checkpoint 2026-08-29 07:34 +08:00

- `mr` 128 KiB completed byte-exactly, bringing E5 to `297/432` `PASS` rows.
- The same runner is now processing `nci` 32 KiB in `auto-b128-r1`; no failure
  or duplicate run exists.

## Runtime checkpoint 2026-08-29 07:21 +08:00

- E5 remains `296/432`, all durable rows `PASS`, failures `0`.
- Codec PID `19888` is CPU-active on `auto-b128-r1` / `mr` / 128 KiB encode.
- Keep the same runner and package; no summary or replacement run exists yet.

## Runtime checkpoint 2026-08-29 07:19 +08:00

- `mr` 64 KiB completed byte-exactly, bringing E5 to `296/432` `PASS` rows.
- `auto-b128-r1` has 8/36 durable rows and is now encoding `mr` 128 KiB.

## Runtime checkpoint 2026-08-29 07:13 +08:00

- E5 reached `295/432` durable `PASS` rows with zero failures.
- The same single runner advanced to the next case under `auto-b128-r1`.

## Runtime checkpoint 2026-08-29 07:11 +08:00

- `mr` 32 KiB completed with byte-exact `PASS`; E5 reached `294/432` rows.
- The same runner continues in fixed order under `auto-b128-r1`; no failure or
  duplicate run was observed.

## Runtime checkpoint 2026-08-29 07:10 +08:00

- `mozilla` 128 KiB completed with a 11,547-byte archive and the runner moved
  to `mr` 32 KiB under `auto-b128-r1`.
- E5 remains at `293/432` durable `PASS` rows until the new case decodes and
  commits its row; no failure or duplicate runner exists.

## Benchmark provenance audit 2026-08-29 07:05 +08:00

- The Silesia leading-prefix manifest validates at 36 rows for 12 files and
  scopes 32/64/128 KiB; all source paths exist and the manifest SHA-256 is
  `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D`.
- The same-input PAQ8px v216 `-1` baseline package validates at 36/36 `PASS`.
- Provenance and distribution boundaries are present in
  `docs/DATASET_PROVENANCE.md`; zstd v1.5.7 donor identity is recorded in
  `docs/provenance/zstd-v1.5.7.json`.

## Partial evidence checkpoint 2026-08-29 07:03 +08:00

- The 293 recorded E5 rows pass status, roundtrip, positive archive length, and
  input/decoded SHA-256 equality checks.
- All four policies and all three scopes are represented in the partial rows;
  aggregate bytes and timings remain provisional until all 432 rows complete.

## Runtime checkpoint 2026-08-29 07:00 +08:00

- E5 has `293/432` durable `PASS` rows and zero failure/error rows.
- PID `30912` is the sole parent runner; child PID `18468` is active on the
  `auto-b128-r1` `mozilla` 128 KiB Auto encode.
- `summary.json` is still absent. Preserve the same package and resume path.

## Runtime checkpoint 2026-08-29 06:58 +08:00

- E5 is still `292/432`, all rows byte-exact `PASS`; the runner has advanced
  from `mozilla` 64 KiB to `mozilla` 128 KiB in `auto-b128-r1`.
- PID `30912` remains the only parent runner and codec child PID `16800` is
  active. `summary.json` is not present.

## Runtime checkpoint 2026-08-29 06:56 +08:00

- E5 remains at `292/432` rows, all byte-exact `PASS`, with zero failures.
- The sole parent runner is PID `30912`; codec child PID `14224` is CPU-active
  on `auto-b128-r1` / `mozilla` / 64 KiB.
- `summary.json` is not present. Keep the same experiment ID and defer E5
  analysis, docs, commits, and E6 runtime until completion.

## E6 preflight checkpoint (2026-08-29 06:53 +08:00)

- Read-only E6 matrix preflight passed for 12 Silesia files, scopes 32/64/128
  KiB, and block sizes 32/64/128 KiB.
- Fast uses one warmup plus three retained repeats: 432 case rows and 864
  codec invocations in the planned package.
- No E6 directory or codec process was created; wait for the active E5 package.

## Runtime checkpoint 2026-08-29 06:55 +08:00

- E5: 290/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 and K=8 b64 groups are complete; `auto-b128-r1` is active under PID
  30912 on `dickens` 128 KiB encode.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 06:40 +08:00

- E5: 290/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- The final b128 group is active under PID 30912; `dickens` 128 KiB encode is
  in progress after its 64 KiB row passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 06:25 +08:00

- E5: 289/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=8 b64 is complete; the final b128 group is active under PID 30912 on
  `dickens` 64 KiB after its 32 KiB row passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 06:23 +08:00

- E5: 288/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=8 b64 is complete (36/36); the final b128 group is active under PID
  30912, currently on `dickens` 32 KiB.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 06:16 +08:00

- E5: 287/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912 on the final `xml` 128 KiB
  encode after its 64 KiB row passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 06:14 +08:00

- E5: 285/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912 on `xml` 32 KiB after
  `x-ray` 128 KiB passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 06:07 +08:00

- E5: 282/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912 on `x-ray` 32 KiB after
  `webster` 128 KiB passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 06:02 +08:00

- E5: 279/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912 on `webster` 32 KiB after
  `sao` 128 KiB passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:43 +08:00

- E5: 269/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912, with `osdb` 128 KiB
  decode in progress after encoding completed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:39 +08:00

- E5: 269/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912 on `osdb` 128 KiB after
  its 64 KiB row passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:36 +08:00

- E5: 267/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912 on `osdb` 32 KiB after
  `ooffice` 128 KiB passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:31 +08:00

- E5: 265/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912, with `ooffice` 64 KiB
  decode in progress after its 32 KiB row passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:29 +08:00

- E5: 264/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912 on `ooffice` 32 KiB after
  `nci` 128 KiB passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:26 +08:00

- E5: 261/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912 with `nci` 32 KiB decode
  in progress after `mr` 128 KiB passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:23 +08:00

- E5: 260/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912, with `mr` 128 KiB encode
  in progress after its 64 KiB row passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:20 +08:00

- E5: 258/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912 on `mr` 32 KiB after
  `mozilla` 128 KiB passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:15 +08:00

- E5: 255/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- K=4 is complete; K=8 is active under PID 30912, with `mozilla` 32 KiB
  decode in progress after `dickens` 128 KiB passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:12 +08:00

- E5: 254/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- `auto-k4-b64-r1` is complete; `auto-k8-b64-r1` is active on `dickens` 128
  KiB after its 64 KiB row passed.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:10 +08:00

- E5: 252/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- `auto-k4-b64-r1` is complete (36/36); PID 30912 is running
  `auto-k8-b64-r1` on `dickens` 32 KiB.
- `summary.json` is pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:06 +08:00

- E5: 249/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 remains the sole runner; `xml` 32 KiB is the next case under
  `auto-k4-b64-r1`.
- `summary.json` is pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 05:02 +08:00

- E5: 247/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 remains the sole runner; `x-ray` 128 KiB encode is active under
  `auto-k4-b64-r1`.
- `summary.json` is pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 04:59 +08:00

- E5: 246/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 remains the sole runner; `x-ray` 32 KiB encode is active under
  `auto-k4-b64-r1`.
- `summary.json` is pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 04:55 +08:00

- E5: 243/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 remains the only runner; `webster` 32 KiB decode is active under
  `auto-k4-b64-r1`.
- `summary.json` is pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 04:50 +08:00

- E5: 242/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 remains the only runner; `sao` 128 KiB encode is active under
  `auto-k4-b64-r1`.
- `summary.json` remains pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 04:46 +08:00

- E5: 240/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 remains the sole runner; `sao` 32 KiB encode is active under
  `auto-k4-b64-r1`.
- `summary.json` is still pending; resume the same experiment ID if needed.

## Runtime checkpoint 2026-08-29 04:43 +08:00

- E5: 239/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 is the sole runner; `samba` 128 KiB encode is active under
  `auto-k4-b64-r1`.
- `summary.json` is pending; no duplicate matrix was started.

## Runtime checkpoint 2026-08-29 04:40 +08:00

- E5: 237/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 remains the only runner; `samba` 32 KiB encode is active under
  `auto-k4-b64-r1`.
- `summary.json` is pending; resume the same experiment ID if interrupted.

## Runtime checkpoint 2026-08-29 04:38 +08:00

- E5: 236/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 remains the sole runner; `reymont` 128 KiB encode is active under
  `auto-k4-b64-r1`.
- `summary.json` is still pending; preserve the same experiment identity.

## Runtime checkpoint 2026-08-29 04:37 +08:00

- E5: 235/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- The sole runner is PID 30912; active child is `reymont` 64 KiB encode under
  `auto-k4-b64-r1`.
- `summary.json` remains pending; no duplicate run or parameter change.

## Runtime checkpoint 2026-08-29 04:32 +08:00

- E5 has 233/432 completed rows, all byte-exact `COMPLETE/PASS`, failures: 0.
- The sole runner is PID 30912; `osdb` 128 KiB is the active Auto encode.
- `summary.json` is not present; preserve and resume the same experiment ID.

## Runtime checkpoint 2026-08-29 04:30 +08:00

- E5: 232/432 rows complete, all byte-exact `COMPLETE/PASS`, failures: 0.
- PID 30912 is the sole runner; current child is `osdb` 64 KiB encode under
  `auto-k4-b64-r1`.
- `summary.json` is pending; the existing ledger remains the only resumable
  experiment.

## Runtime checkpoint 2026-08-29 04:26 +08:00

- E5 matrix `hybridzip-r2-e5-router-320dd1b-v1`: 230/432 rows complete.
- All recorded rows are byte-exact `COMPLETE/PASS`; failures: 0.
- PID 30912 remains the only authorized runner; its codec child is encoding
  `ooffice` 128 KiB under `auto-k4-b64-r1`.
- `summary.json` is still pending and no duplicate run was launched.

## Attachment-Driven Experiment Charter - 2026-08-28

- Source: `C:/Users/Administrator/.codex/attachments/b96760cb-802c-4f54-b886-1fce9454f953/pasted-text.txt`.
- Decision retained: one HZ02 wire contract; HZ01 decoding; IDs `0..42` fixed;
  ID `43` remains the append-only Fast extension.
- Measurement separation retained: `ENC_RATIO_V1` uses same-input complete
  archive bytes against PAQ8px; `ENC_FAST_V1` uses CPU/GPU throughput. Neither
  policy result proves the other.
- Current first execution target is the 12-file, 32 KiB forced oracle: Auto
  plus 43 forced modes, 44 child packages, and 1,056 encode/decode invocations
  before retries. Its no-codec preflight is permitted; runtime must use the
  explicit authorization switch and a new ledger ID.
- Preflight result: current Release codec SHA-256 is
  `8E64B93362D4A0C9EBC9C81052839A4966AE502A9CCB53F876E21BF4D5C4B4E7`;
  the 36-row manifest retains SHA-256
  `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D`;
  all 12 Silesia sources are present and at least 128 KiB. The list-only
  oracle plan reported `runtime_started=false`. Synthetic tied-winner
  derivation and AST parsing of the three runtime scripts passed.
- K=8 remains experimental until a current-build forced ledger supplies
  file-level no-leakage labels and held-out tie-aware recall/regret passes.
- OASum remains blocked on a separate owner/legal decision: its complete
  `test.jsonl` is 1,065,019,104 bytes and CC-BY-SA-3.0.

## PAQ8px RecordModel Short-Block Repair - 2026-08-28

- The focused current-build test initially crashed in forced Mode 32 on the
  23-byte `"abracadabra abracadabra"` regression input. GDB located the
  access violation in donor `ContextMap::set()` beneath `RecordModel::mix()`.
- Cause: `Paq8pxRecordModelBackend::context_table_size()` allowed a 64-byte
  table. `ContextMap` represents that as one 64-byte bucket; `hashBits` became
  zero and the donor's `finalize64()` performed an undefined 64-bit shift,
  allowing an out-of-range bucket index.
- Repair: set the backend minimum to 128 bytes, giving the donor at least two
  buckets. The HZ02 mode ID and wire format are unchanged. Archives that used
  the defective one-bucket encoder were never reliable, so no valid legacy
  stream contract is weakened.
- Verification: current Release codec SHA-256
  `74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F`
  encoded and decoded the previously crashing 24-byte input byte-exactly
  (84-byte archive); `hz_r2_codec_tests` and `hz_structure_routing_tests`
  passed. The first now explicitly covers 1-byte and 64-byte forced Mode-32
  round trips. The repaired decoder also byte-exactly decoded retained HZ01
  and previous 1-KiB Mode-32 archives. Evidence:
  `results/smoke/r2-current-record-model-24-20260828-fixed/verification.json`.
- Consequence: the prior E4 preflight was for codec SHA-256 `8E64...B4E7`.
  It must be run again after this repair, still in list-only mode, before an
  authorized forced-oracle ledger can start.

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

## 2026-08-28: F3 implementation boundary

- Attachment source: `C:/Users/Administrator/.codex/attachments/b96760cb-802c-4f54-b886-1fce9454f953/pasted-text.txt`.
- F3 is applicable now because the pre-change single-thread Fast baseline is
  already measured. It requires HybridZip-owned block parallelism while zstd
  remains configured with zero internal workers.
- `BlockPlanner` has mutable family telemetry for full Auto, so parallelizing
  arbitrary policies would change semantics. Fast exits before telemetry and
  can safely use one planner per worker.
- The executor must cap in-flight raw blocks, retain result indices, and emit
  headers, CRC, extension metadata, and payload in ascending input block
  order. It must not add a thread count to HZ02 archive bytes.
- The only current runtime authorization is a deterministic 1 KiB input split
  into four 256-byte blocks, comparing one and two Fast workers. The required
  outcomes are byte-exact decoding and equal archive SHA-256 values. Timing
  claims remain deferred to a dedicated post-change E6 package.
- F3.2 passed on the final Release SHA-256
  `B7B9AB415D5E10A060F563C2E85B5A436D563B10FCAAFE3687BBD69D9D89DB53`.
  Both one and two workers produced the same 690-byte HZ02 archive SHA-256
  `1348E55F39324AF742909BF6B662647D20A07B58981DDF8A7BDA70B24CCC6DFD`.
  Both decoded hashes equal the 1,024-byte input hash
  `AE3D26437535BB17D778294E8973030B49A7EA5337E3183532FCE00C649EB6D7`.
  The Fast policy evaluated 16 candidates over four blocks and selected Mode
  43 on each. Evidence:
  `results/smoke/r2-f3-fast-executor-1k-20260828-v2/verification.json`.
- F3.4a adds `-FastThreadCount` to `tools/run_r2_e5_e6_matrix.ps1` and
  `-ThreadCount` to the child runner. The parent stores the requested count in
  its experiment metadata, rejects incompatible resume packages, forwards it
  to `--threads` for Fast, checks `workers=` encoder telemetry, and includes
  both values in matrix/summary rows. E5 and every non-Fast child reject a
  count above one. PowerShell AST parsing passed; E6 one-file/one-cell
  `-ListOnly` preflights passed at one and two workers with
  `runtime_started=false`.

## 2026-08-28: F1 model identity telemetry

- The canonical fixed-point ranker image contains little-endian weights,
  biases, feature shifts, version, and CRC32, totaling 2,644 bytes. Its frozen
  bootstrap identity is `0x00010000` / `0x1025B343` /
  `4B1AC26C40AD4DA50312FD3B694D7E636FB768C2336FE773BC82D36424C27A4B`.
- `mode_ranker` reaches the existing libzpaq SHA-256 implementation through
  the established ZPAQ adapter. This preserves the donor callback linkage and
  avoids a second hash implementation.
- R2 CLI telemetry now emits the identity; the E5/E6 matrix runner records it
  per row, adds it to summaries, and refuses completed packages with a mixed
  identity set.
- Release build and `hz_structure_routing_tests.exe` passed. The retained
  random 1 KiB `auto-k8` smoke produced a 1,084-byte archive and matching
  input/decoded SHA-256. Evidence:
  `results/smoke/r2-f1-model-identity-1k-20260828-v2/verification.json`.
- A first PowerShell xorshift generator overflowed during input creation and
  never launched the codec. It is excluded; the retained v2 input uses the
  system random-number source.

## 2026-08-28: Forced-mode oracle label infrastructure

- `run_r2_complete_ledger.ps1` now records and forwards `block_size_kib`.
  The dedicated ratio-label run uses 32 KiB scope and 32 KiB internal blocks,
  making each archive result an exact one-block forced comparison.
- `derive_r2_forced_oracle.ps1` validates the 43 retained ratio modes, same
  executable/input identity, complete archive bytes, and exact forced block
  attribution. It preserves all tied minimum-byte modes and evaluates whether
  an E5 K2/K4/K8 candidate set contains any tied winner.
- `test_r2_forced_oracle.ps1` passed without invoking the codec. Its synthetic
  oracle ties `zstd,fse`; K2 misses and K4/K8 hit, proving tie handling and
  candidate-ID joins. The tool rejects mixed ranker identities and incomplete
  coverage when requested.
- Runtime remains deliberately pending. The full 12-file 32 KiB forced ledger
  has 44 child packages and 1,056 codec invocations before retries.

## 2026-08-28: E5 forced-oracle evidence binding

- `tools/run_r2_e5_e6_matrix.ps1` now accepts E5-only
  `-ForcedOracleLedgerPath`. It writes the normalized ledger path and planned
  `forced-oracle` output location into `experiment.json`; a resume request
  with a different path is rejected.
- Runtime use first invokes `derive_r2_forced_oracle.ps1 -ListOnly` to validate
  the completed forced ledger before launching E5. After all E5 rows pass, it
  derives tied-winner recall under `<e5-package>\forced-oracle` and embeds the
  evidence link in E5 `summary.json`.
- Verification used no codec process: PowerShell AST parsing passed;
  `tools/test_r2_forced_oracle.ps1` passed its synthetic `zstd,fse` tie; an E5
  one-file/32 KiB `-ListOnly` plan reported four child packages and eight
  planned codec invocations with `runtime_started=false`.
- The implementation and updated execution design are committed and pushed as
  `996d59d` (`feat(r2): bind forced oracle to E5`).

## 2026-08-28: zstd v1.5.7 donor provenance reconciliation

- The warehouse already contains the official v1.5.7 release archive at
  `E:/MIXER/KU/zstd-v1.5.7/zstd-1.5.7.tar.gz`; it is 2,434,947 bytes with
  SHA-256 `EB33E51F49A15E023950CD7825CA74A4A2B43DB8354825AC24FC1B7EE09E6FA3`.
- Upstream tag verification returns annotated tag object
  `ac66b19e6bd6b83238bf008eecc1298105298532` and peeled release commit
  `f8745da6ff1ad1e7bab384bd1f9d742439278e99`. Its extracted 639-file source
  tree identifies as zstd 1.5.7; the selected BSD-3-Clause `LICENSE` SHA-256
  is `7055266497633C9025B777C78EB7235AF13922117480ED5C674677ADC381C9D8`.
- The durable repository record is `docs/provenance/zstd-v1.5.7.json`. It
  explicitly marks the donor staged, not imported: production still compiles
  the separately recorded vendored zstd 1.6.0.

## 2026-08-28: K=8 no-leakage training data interface

- `hz_r2_feature_dump.exe` is a read-only target linked to the production C++
  `BlockFeaturesV1` extractor and ranker. On the retained random 1 KiB input it
  emitted 28 values, eight K=8 mode IDs, and the pinned model SHA-256
  `4B1AC26C40AD4DA50312FD3B694D7E636FB768C2336FE773BC82D36424C27A4B`.
- `export_r2_ranker_training_set.ps1` accepts only a `COMPLETE`, 32 KiB
  forced-oracle package. It verifies every source-prefix SHA-256, requires an
  explicit non-empty file-level holdout, and writes source hashes, tied winner
  labels, 28 exact C++ features, K=8 candidate IDs, and model identity.
- `test_r2_ranker_training_set.ps1` passed with two synthetic 32 KiB sources:
  the preview launched no feature dump or codec, the completed export invoked
  only the feature tool, and its single validation file was absent from
  training. No HybridZip archive was encoded or decoded.
- The current-worktree `hz_structure_routing_tests.exe` was rebuilt and run
  after this interface was added. It passed and exercises deterministic
  feature extraction, class routing, K=2/K=4/K=8 cardinality and membership,
  plus the pinned fixed-point ranker identity without archive construction.

## 2026-08-28: Attachment latency telemetry design

- The attachment requires P50/P95 block queue-plus-service and service-only
  latency for the Fast policy. The existing executor had canonical-order
  completion and worker-count telemetry but exposed only file-level timing.
- The new evidence contract records a timestamp at bounded-queue enqueue and a
  timestamp after `BlockPlanner::plan`. Queue-plus-service is their difference;
  service-only starts immediately before `plan`. Neither metric is archive
  metadata, and neither includes input reads or canonical archive writes.
- The matrix runner stores every raw nanosecond block sample in `matrix_rows.csv`
  and calculates P50/P95 from the concatenated samples in each E6 summary row.
  This avoids treating a file-level elapsed time as a block-latency proxy.
- The next permitted verification is one 1 KiB input split into four 256-byte
  blocks at one and two workers. The E5 forced oracle and post-change E6 corpus
  matrices remain unstarted.
- The retained smoke used a 1,024-byte deterministic input and current codec
  SHA-256 `8E64B93362D4A0C9EBC9C81052839A4966AE502A9CCB53F876E21BF4D5C4B4E7`.
  One and two workers both produced a 540-byte archive with SHA-256
  `88CD5B6460E21CBF8E86592BFDDD35D2CF212EB78160D6411A7BE1D0714DACE2`.
  Both decoded hashes matched input SHA-256
  `E051D1007607DE494C073DA3C29903D6C0ABFEE7A4C0609F560A340A1947B470`;
  each run emitted four paired timing samples. No throughput claim is made.
- The E6 telemetry parser now recomputes nearest-rank P50/P95 from the raw
  samples before retaining a row. It rejects mismatched percentiles, unequal
  paired sample counts, nonzero percentiles for empty samples, and any
  queue-plus-service sample below its paired service-only value. PowerShell AST
  parsing passed; the retained 1 KiB log passed and a P50-tampered copy was
  rejected without launching the codec.

## 2026-08-28: Benchmark environment identity

- `tools/capture_r2_environment.ps1` records a structured environment snapshot
  without enumerating user environment variables. It includes OS, CPU topology,
  RAM, GPU/driver when WMI can discover it, active power plan, compiler version,
  codec SHA-256, and Git revision/dirty state. The fingerprint excludes the
  capture timestamp.
- New E5/E6 matrix packages and forced-oracle ledgers write `environment.json`.
  Resume recomputes the fingerprint and rejects a changed benchmark host or
  source state. Child experiment metadata now records an actual source revision
  instead of the former `working-tree-uncommitted` placeholder.
- On the current host two captures produced the identical fingerprint
  `D2361A6DBEA69EC701710515FB46651EE4A5CBE5F5EC2ACEE20F84E99E87607D`; the
  capture included one CPU and one GPU entry plus power-plan and compiler data.
  A second write to the same manifest path was rejected. No codec was launched.

## 2026-08-28: Candidate ranker fitter

- `tools/fit_r2_fixed_point_ranker.py` now consumes only a COMPLETE exported
  file-level training package. It validates `summary.json`, `split.json`, CSV
  schema, one row per source file, tied winner names, and the zero-codec
  boundary before fitting.
- The fitter uses deterministic multiclass perceptron updates over the exact
  28-feature shift contract, then writes a 2,644-byte little-endian candidate
  image with CRC32/SHA-256 and a complete fit manifest. The model is marked
  `CANDIDATE_FROZEN_NOT_INSTALLED`; no C++ source or active ranker is changed.
- `tools/test_fit_r2_fixed_point_ranker.py` passed: two independent fits of a
  synthetic two-training/one-validation split produced identical model bytes,
  and the validation row remained isolated. Python compilation passed. No
  codec process was launched.

## 2026-08-28: Current-Commit E4 Preflight

- User direction: after publishing the current version, apply the attached R2
  decision to experiment design and begin target execution.
- Publication check: local `main`, upstream `origin/main`, and `HEAD` equal
  commit `0670cc389f054d3966eb5acfa029729ca72ad6ae` (`fix(r2): guard record
  model short blocks`). The branch is publicly available at
  `https://github.com/KennethParkerWang/HybridZip`.
- Current Release executable identity:
  `74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F`.
- Read-only E4 invocation:

  ```powershell
  .\tools\run_r2_complete_ledger.ps1 `
    -CodecPath .\build\Release\hybridzip.exe `
    -DatasetPath F:\paq8px\silesia `
    -OutputRoot .\results\experiments `
    -LedgerId hybridzip-r2-forced-oracle-current-0670cc3 `
    -ScopesKiB 32 -BlockSizeKiB 32 -ListOnly
  ```

- Observed plan: 12 frozen files; 44 modes (Auto plus 43 forced modes); 32 KiB
  input/internal block; 1,056 planned codec invocations; `runtime_started=false`.
- The preflight did not create or overwrite a runtime package and did not start
  a codec process. The next runtime action is deliberately separated because
  E4 is PAQ-heavy.

## 2026-08-28: Public Evidence Boundary Repair

- README previously described the historical `CC6DA840...BF191` 528-row R2
  ledger as belonging to the active Release, even though it predates Mode 43
  and the current RecordModel short-block repair.
- `README.md` and `docs/PRODUCT_STATUS.md` now name the current Release hash
  `74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F`, retain
  the historical ledger as provenance, and prohibit current PAQ-ratio, K=8
  regret, or post-change Fast-throughput claims until their runtime gates pass.
- README now specifies the correct container contract: routing is encoder-only;
  selected mode and reversible metadata are decoder-visible.
# Resume checkpoint: E4 forced-oracle (2026-08-28 13:19 +08:00)

- The authorized E4 runner is still active under ledger ID
  `hybridzip-r2-forced-oracle-current-320dd1b`; parent PowerShell PID 17276
  started at 12:45 and owns the run.
- Scope is fixed at 12 Silesia leading prefixes, 32 KiB input, 32 KiB internal
  blocks, Auto plus 43 forced modes: 44 packages and 1,056 planned
  encode/decode invocations before retries.
- The ledger manifest is
  `results/analysis/r2-complete-ledger/hybridzip-r2-forced-oracle-current-320dd1b/manifest.tsv`.
  Auto and stored are `COMPLETE`, predictive is `TESTING`, and the remaining
  41 forced packages are `PENDING`; no failure is recorded. The current child
  is processing `predictive/dickens.bin` and writes a `.tmp` archive until
  encode completes.
- A session/quota interruption is recoverable with the same `LedgerId` and
  `-Resume`; the runner validates codec hash, environment fingerprint,
  manifest dimensions, and existing package rows before continuing.
- Do not start E5/E6 or create another E4 ledger until this package reaches
  `COMPLETE/PASS` and forced-oracle labels are derived.

## Runtime progress checkpoint: 2026-08-28 13:37 +08:00

- The same E4 parent runner remains active (PID 17276).
- Manifest status is `COMPLETE=6`, `TESTING=1`, `PENDING=37`; the testing
  package is `donor-match`.
- No new ledger or duplicate codec run was started. Existing package outputs
  remain the recovery source for `-Resume`.

## Runtime progress checkpoint: 2026-08-28 13:45 +08:00

- The original E4 parent runner is still active.
- Manifest status is `COMPLETE=22`, `TESTING=1`, `PENDING=21`; the testing
  package is `neural-lstm`.
- No failure row, second ledger, or duplicate codec run was observed.

## Runtime progress checkpoint: 2026-08-28 13:49 +08:00

- Manifest status is `COMPLETE=23`, `TESTING=1`, `PENDING=20`; the testing
  package is `shared-neural-lstm`.
- The original E4 parent runner remains active. Post-E4 work is ordered as
  forced-oracle derivation, no-leakage export, uninstalled ranker fitting, E5,
  then post-change E6.

## Runtime progress checkpoint: 2026-08-28 13:53 +08:00

- `shared-neural-lstm` has 6/12 `COMPLETE/PASS` rows while its package is
  still `TESTING`; the codec process remains active.
- The top-level manifest is unchanged at `COMPLETE=23`, `TESTING=1`,
  `PENDING=20`. Partial package rows remain on disk for same-ID recovery.

## Runtime progress checkpoint: 2026-08-28 13:56 +08:00

- The `shared-neural-lstm` package has 9/12 completed file rows; its codec
  child is still active and the top-level manifest remains in `TESTING`.

## Runtime progress checkpoint: 2026-08-28 14:01 +08:00

- `shared-neural-lstm` completed and `lstm-compress` is now `TESTING`.
- Top-level status is `COMPLETE=24`, `TESTING=1`, `PENDING=19`; no failure
  row or duplicate ledger was observed.

## Runtime progress checkpoint: 2026-08-28 14:06 +08:00

- Top-level status is `COMPLETE=27`, `TESTING=1`, `PENDING=16`.
- The latest completed packages are `lstm-compress`, `delta-of-delta-zstd`,
  and `bgpt-shared-prior`; `jax-compress-portable` is currently testing.
- No failures or duplicate runs were observed; E5/E6 remain stopped.

## Runtime progress checkpoint: 2026-08-28 14:07 +08:00

- Top-level status is `COMPLETE=31`, `TESTING=1`, `PENDING=12`.
- `jax-compress-portable`, `ppmd7`, `ppmd8`, and `zpaq` completed; `ctw` is
  currently testing.
- No failures or duplicate runs were observed.

## Runtime progress checkpoint: 2026-08-28 14:09 +08:00

- Top-level status is `COMPLETE=32`, `TESTING=1`, `PENDING=11`.
- `ctw` completed and `paq8px-apm` is currently testing; no failure row was
  recorded.

## Runtime progress checkpoint: 2026-08-28 14:11 +08:00

- `paq8px-apm` is at 4/12 completed file rows and remains CPU-active.
- The top-level manifest remains `COMPLETE=32`, `TESTING=1`, `PENDING=11`; no
  restart, failure, or duplicate ledger was observed.

## Runtime progress checkpoint: 2026-08-28 14:14 +08:00

- `paq8px-apm` reached 9/12 completed rows and remains CPU-active.
- Top-level status is unchanged at `COMPLETE=32`, `TESTING=1`, `PENDING=11`;
  no restart or failure was observed.

## Runtime progress checkpoint: 2026-08-28 14:16 +08:00

- Top-level status is `COMPLETE=35`, `TESTING=1`, `PENDING=8`.
- `paq8px-apm` and `paq8px-linear-prediction` completed; the testing package
  is `paq8px-similarity`.
- No failure row or duplicate ledger was observed.

## Runtime progress checkpoint: 2026-08-28 14:20 +08:00

- Top-level status is `COMPLETE=37`, `TESTING=1`, `PENDING=6`.
- `paq8px-similarity-sse` completed; `paq8px-generic-sse` is currently
  testing.
- No failure row or duplicate ledger was observed.

## Runtime progress checkpoint: 2026-08-28 14:23 +08:00

- Top-level status is `COMPLETE=38`, `TESTING=1`, `PENDING=5`.
- `paq8px-generic-sse` completed and `paq8px-detected-sse` is currently
  testing.
- No failure row or duplicate ledger was observed.

## 2026-08-28: E4 forced-oracle completion and recovery point

- The resumed ledger
  `results/analysis/r2-complete-ledger/hybridzip-r2-forced-oracle-current-320dd1b/`
  is complete: 44/44 packages are `COMPLETE`, with Auto plus 43 forced ratio
  modes over 12 Silesia 32 KiB prefixes and 32 KiB internal blocks.
- The run produced no failure rows and no active `hybridzip` process remains.
  Existing per-mode package directories are the recovery source; they must not
  be deleted or replaced by a new E4 ledger.
- Forced-oracle derivation is complete at
  `results/analysis/r2-forced-oracle-derived-320dd1b/`. It contains
  `forced_archive_rows.csv`, `forced_oracle_rows.csv`, and `summary.json`.
  The summary records 43 ratio modes, 12 input cases, codec SHA-256
  `74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F`, and
  no E5 package yet.
- The next bounded action is the no-codec training export using
  `tools/export_r2_ranker_training_set.ps1`, with `webster`, `x-ray`, and
  `xml` held out at file level. The exporter reads the complete forced-oracle
  package and calls only `hz_r2_feature_dump.exe`; it must not install or
  modify the production ranker.
- Recovery rule: keep the exact output directory and inspect its manifest
  after interruption. Resume only the export/fitter step before starting E5.

## 2026-08-28: Candidate ranker fit checkpoint

- Real no-leakage training data is complete at
  `results/analysis/r2-ranker-training-320dd1b-v1/`: 9 training rows and 3
  file-level validation rows (`webster`, `x-ray`, `xml`). The exporter called
  the read-only feature tool 12 times and launched no codec process.
- The deterministic offline fitter produced
  `results/analysis/r2-ranker-fit-320dd1b-v1/` with candidate model CRC32
  `A0354863` and SHA-256
  `CA1B144EF35E20EC388D739ACE9A1EF92A5E72410D050B5021C3A7F93C62D7B3`.
  Validation top-1 tied-winner recall is `1.0` on the three held-out files.
- The model remains candidate-only (`CANDIDATE_FROZEN_NOT_INSTALLED`). This
  validation metric is not the E5 K=8 shortlist recall or archive-byte regret
  gate; those require the guarded runtime matrix.
- Next recovery point is the E5 `-ListOnly` preview. Do not start a second
  forced-oracle ledger and do not install the candidate model before E5.

## 2026-08-28: E5 runtime checkpoint

- The E5 router preview passed with 12 child packages, 36 cases per child,
  and 864 planned codec invocations. It uses the complete forced-oracle ledger
  `hybridzip-r2-forced-oracle-current-320dd1b` as its attribution source.
- The authorized runtime is now assigned the stable ID
  `hybridzip-r2-e5-router-320dd1b-v1`; its package root is
  `results/experiments/hybridzip-r2-e5-router-320dd1b-v1/`. The runner writes
  each child package incrementally and skips validated `COMPLETE` children on
  `-Resume`.
- E5 evaluates the checked-in encoder policies and records their production
  ranker telemetry. The fitted `00010001` candidate image remains offline and
  is not installed; the runtime must not be described as candidate-model
  evidence unless its model identity is present in the telemetry.
- The first runtime launch was rejected at the explicit-scope guard because
  `-SilesiaFiles`/`-AllowAllFiles` was omitted. It exited before package
  creation and before any codec process. The corrected command explicitly
  lists all 12 frozen files and reuses the same stable ExperimentId.
- Recovery command is recorded in `task_plan.md`. On interruption, inspect
  `experiment.json` and child package states, then rerun the same ID with
  `-Resume`; never create a replacement E5 ledger.

## 2026-08-28 15:12: E5 runtime progress

- The corrected explicit-file launch is active under
  `hybridzip-r2-e5-router-320dd1b-v1`. Environment fingerprint is
  `6DC773B755B24DBBD0273C4A9E798DF9911284DDE2CFC66E5C7BE4A161D79D5D` and
  codec identity remains `74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F`.
- One codec child is active. Child `auto-b32-r1` has completed its first
  `dickens` 32 KiB row with `COMPLETE/PASS`; the parent session is still
  running and no duplicate task was started.

## 2026-08-28 15:45: E5 runtime progress

- The first E5 child `auto-b32-r1` completed all three `dickens` scopes
  (32/64/128 KiB) with `COMPLETE/PASS` and byte-exact reconstruction.
- The runner continues in the same session and stable ExperimentId. Partial
  output remains in `results.csv`; resume must use the same ID and never create
  a replacement package.

## 2026-08-28: E5 six-row checkpoint

- `auto-b32-r1` completed `dickens` and `mozilla` at 32/64/128 KiB (6/36
  rows), all byte-exact `PASS`.
- The runner moved to the next file with one active codec process. The child
  CSV remains the durable progress source for any later `-Resume`.

## 2026-08-28: E5 twelve-row checkpoint

- `auto-b32-r1` completed all scopes for four files (`dickens`, `mozilla`,
  `mr`, `nci`), totaling 12/36 rows, all byte-exact `PASS`.
- The existing parent session continues with the same package and stable ID;
  no duplicate runtime was launched.

## 2026-08-28: E5 fourteen-row checkpoint

- The first child has 14/36 completed rows: four files at 32/64/128 KiB and
  `ooffice` at 32/64 KiB. All rows passed byte-exact round-trip checks.
- The parent continues the same stable E5 package with no duplicate codec
  process; the child CSV remains the recovery source.

## 2026-08-28: E5 twenty-four-row checkpoint

- `auto-b32-r1` completed eight files through `samba` at 32/64/128 KiB,
  totaling 24/36 rows, all byte-exact `PASS`.
- The same parent session advances to `sao`; no duplicate runtime was started.

## 2026-08-28: E5 twenty-seven-row checkpoint

- `auto-b32-r1` completed nine files through `sao` at all three scopes,
  totaling 27/36 rows, all byte-exact `PASS`.
- Only `webster`, `x-ray`, and `xml` remain in this child before the next
  block-size/policy child starts.

## 2026-08-28: E5 thirty-row checkpoint

- `auto-b32-r1` completed ten files through `webster` at 32/64/128 KiB,
  totaling 30/36 rows, all byte-exact `PASS`.
- Only `x-ray` and `xml` remain before this child is finalized.

## 2026-08-28: E5 thirty-four-row checkpoint

- `auto-b32-r1` completed `x-ray` at 32/64/128 KiB and `xml` at 32 KiB,
  totaling 34/36 rows, all byte-exact `PASS`.
- The newly attached HZ03 decision has been read but is not yet adopted as an
  implementation instruction because the user message contained no explicit
  action request. E5 continues under the existing R2 evidence boundary.

## 2026-08-28: E5 first child complete

- `auto-b32-r1` completed all 36 frozen cases (12 files x 32/64/128 KiB),
  all byte-exact `COMPLETE/PASS`.
- The parent runner finalized the child and advanced to `auto-k2-b32-r1`.
  Existing child artifacts are immutable recovery evidence; no duplicate run
  was created.

## 2026-08-28: E5 second child nine-row checkpoint

- `auto-k2-b32-r1` completed the first three files (`dickens`, `mozilla`,
  `mr`) across 32/64/128 KiB, totaling 9/36 rows, all byte-exact `PASS`.
- The parent continues under the same stable package. A future `-Resume`
  validates and skips the finalized `auto-b32-r1` child.

## 2026-08-28: E5 second child complete

- `auto-k2-b32-r1` completed all 36 frozen cases (12 files x 32/64/128 KiB),
  all byte-exact `COMPLETE/PASS`.
- The parent advanced to `auto-k4-b32-r1`; both finished child packages are
  retained as immutable resume evidence.

## 2026-08-28: E5 third child twenty-one-row checkpoint

- `auto-k4-b32-r1` completed seven files through `reymont` at all three
  scopes, totaling 21/36 rows, all byte-exact `PASS`.
- The same parent runtime is processing `samba`; no duplicate or replacement
  package was created.

## 2026-08-28: E5 three-child checkpoint

- Children `auto-b32-r1`, `auto-k2-b32-r1`, and `auto-k4-b32-r1` are complete,
  each with 36/36 byte-exact `PASS` rows.
- The parent advanced to `auto-k8-b32-r1`, retaining all prior child outputs
  for resume and later aggregate analysis.

## 2026-08-28: E5 K=8 in-progress checkpoint (21:37 +08:00)

- The first three E5 children are finalized with 108 total byte-exact
  `COMPLETE/PASS` rows.
- `auto-k8-b32-r1` is at 29/36 rows, all passing, with no failure/error row.
- One active `hybridzip.exe` belongs to the original E5 runner. No second
  ledger, retry package, or E6 runtime exists.
- Recovery remains the same stable `ExperimentId` and `-Resume` command in
  `task_plan.md`; completed child packages must be skipped, not rerun.

### 21:39 row update

- `auto-k8-b32-r1` reached 31/36 byte-exact `COMPLETE/PASS` rows. The process
  remains active under the original ledger and no failure or duplicate run is
  present.

### 21:40 row update

- `auto-k8-b32-r1` reached 32/36 byte-exact `COMPLETE/PASS` rows. No failure
  row, duplicate process, or replacement ledger was observed.

### 21:44 row update

- `auto-k8-b32-r1` reached 33/36 byte-exact `COMPLETE/PASS` rows, bringing the
  E5 parent to 141 passing rows. The same runner remains active with no error.

## 2026-08-28: E5 block-size 64 KiB started (21:52 +08:00)

- `auto-k8-b32-r1` finalized at 36/36; the parent now contains 144 passing
  rows.
- `auto-b64-r1` started and completed its first `dickens` 32 KiB case, making
  145 total passing rows. No failure/error row or duplicate runtime exists.
- Recovery remains the existing E5 `ExperimentId`; completed children must be
  skipped on resume.

### 21:56 runtime detail

- The first `auto-b64-r1` case remains in its 64 KiB-scope encode phase with a
  `.hz2.tmp` archive and no stderr output. Parent progress is still 145/432
  passing rows; resume must reuse the same experiment ID.

### 21:59 row update

- `auto-b64-r1` reached 2/36 passing rows, bringing the E5 parent to 146/432
  byte-exact rows. The original runner remains active and no duplicate ledger
  or failure row was observed.

## 2026-08-28: E5 22:00 recovery checkpoint

- E5 has 146/432 durable `COMPLETE/PASS` case rows (33.8%), corresponding to
  292/864 encode/decode invocations. Four of twelve child packages are
  finalized; `auto-b64-r1` is at 2/36.
- The original single codec process remains CPU-active and the ledger has no
  failure/error row. Keep the stable ExperimentId for any future resume; do
  not start a replacement matrix.

### 22:09 update

- E5 remains at 146/432 passing rows with `auto-b64-r1` at 2/36; the single
  codec child is processing `dickens` 128 KiB under the existing experiment.
- There are no failures or duplicate ledgers. GitHub push and README updates
  remain intentionally deferred until the E5 summary is complete, as requested.

### 22:14 update

- E5 is unchanged at 146/432 passing rows; `auto-b64-r1` is 2/36. The codec
  remains responsive and CPU-active on `dickens` 128 KiB, with no error log or
  duplicate process.
- The process is intentionally left running; README and GitHub publication
  stay gated on E5 completion.

### 22:17 update

- `auto-b64-r1` reached 3/36 passing rows, for 147/432 total E5 rows. No
  failure/error row or duplicate process exists; the fixed runner continues.

### 22:19 update

- `auto-b64-r1` remains in progress at 3/36; E5 total is 147/432 passing
  rows. The codec is responsive and no failure/error or duplicate run exists.
- No GitHub or README change is made before the E5 summary, per the user
  boundary.

### 22:21 update

- `auto-b64-r1` reached 4/36 passing rows; E5 total is 148/432 and has no
  failure/error row. The original codec process remains responsive.
- GitHub publication and README changes remain gated on final E5 summary.

### 22:22 update

- `auto-b64-r1` is at 4/36; the E5 parent has 148/432 passing rows and no
  failure/error row. The single codec process remains active and responsive.
- The required post-E5 order is unchanged: summary evidence, README, Git
  commit/push, then stop without launching E6 or HZ03.

### 22:27 update

- `auto-b64-r1` reached 5/36 passing rows, bringing E5 to 149/432 total.
  There are no failure/error rows and only the original codec child is active.
- README and GitHub publication remain deferred until E5 writes its final
  summary, as requested.

### 22:40 update

- `auto-b64-r1` completed `mozilla` 128 KiB and reached 6/36 passing rows;
  E5 total is 150/432 with no failure/error row.
- The same runner continues in fixed case order; README and GitHub changes
  remain deferred until final E5 summary.

### 22:43 update

- `mr` 32 KiB completed in `auto-b64-r1`; E5 is 151/432 passing rows, 7/36 in
  the active child, with no failure/error row or duplicate runtime.

### 22:49 update

- `auto-b64-r1` reached 8/36 passing rows and E5 reached 152/432. The `mr`
  64 KiB case completed with 370.660 s encode and 33.995 s decode; no failure
  or duplicate process was observed.

### 23:01 update

- `mr` 128 KiB completed and `auto-b64-r1` reached 9/36; E5 total is 153/432
  passing rows with no failure/error row. The original runner advanced in
  fixed order and no duplicate runtime exists.

### 23:05 update

- E5 reached 154/432 passing rows; `auto-b64-r1` remains active and has no
  failure/error row. No second runner was launched.
- README and GitHub publication remain deferred until E5 summary completion.

### 23:07 update

- E5 remains at 154/432 passing rows with no failure/error row; the original
  runner continues `auto-b64-r1` independently.
- The read-only monitor was stopped to avoid unnecessary polling. No codec,
  README, Git, E6, or HZ03 action was taken beyond the existing E5 runner.

### 23:09 final checkpoint for this turn

- E5 remains at 154/432 passing rows with no failure/error row; the active
  package is `auto-b64-r1`.
- There is one real E5 runner and one codec child. A second match in the raw
  process query was the read-only query shell, not a duplicate experiment.
- Keep the runner unchanged. The post-E5 sequence remains evidence summary,
  README, Git commit/push, then no new experiments.

## 2026-08-28: E5 denominator correction

- The E5 matrix contains 432 result case rows (12 files x 3 scopes x 3 block
  sizes x 4 policies). Each case has one encode and one decode, so the plan
  contains 864 codec invocations.
- An earlier disk state was 155/432 passing case rows, or 310/864 invocations;
  there were no failure/error rows.
- A later disk check reached 158/432 passing case rows, or 316/864
  invocations; there are still no failure/error rows.
- The latest disk check reached 159/432 passing case rows, or 318/864
  invocations; there are still no failure/error rows.
- Historical checkpoint text has been normalized so `X/432` denotes case
  rows, while `864` denotes invocation capacity only.

### 23:46 runtime checkpoint

- E5 has 159/432 `COMPLETE/PASS` case rows, or 318/864 encode/decode
  invocations, with no failure/error row.
- The original runner remains active with one codec child in `auto-b64-r1`;
  `summary.json` has not been produced.
- README and GitHub publication remain deferred until E5 completion; no new
  experiment has been started.

### 23:48 runtime checkpoint

- E5 reached 160/432 passing case rows, or 320/864 encode/decode invocations,
  with no failure/error row.
- The original `auto-b64-r1` runner remains active with one codec child and no
  duplicate experiment. `summary.json` is still absent.

### 23:50 runtime checkpoint

- E5 reached 160/432 passing case rows, or 320/864 encode/decode invocations,
  with no failure/error row.
- The same single runner and codec child continue under `auto-b64-r1`; no
  replacement matrix or new experiment was started.

### 2026-08-29 00:04 runtime checkpoint

- E5 currently contains 161/432 `COMPLETE` rows with `roundtrip=PASS`, or
  322/864 encode/decode invocations; failed and incomplete recorded rows are
  zero.
- Process inspection confirms the original runner PID 30912 and one codec
  child PID 30796. The child is processing `osdb` at 128 KiB with a 64 KiB
  block under `auto-b64-r1`.
- The final `summary.json` has not appeared, so the post-E5 evidence summary,
  README update, and GitHub commit/push remain gated.

### 2026-08-29 00:18 runtime checkpoint

- E5 reached 162/432 `COMPLETE` rows with `roundtrip=PASS`, or 324/864
  encode/decode invocations; no failure or incomplete row is present.
- The long `osdb` 128 KiB / 64 KiB-block case finished successfully, and the
  same runner is now encoding `reymont` 32 KiB under `auto-b64-r1`.
- No duplicate runner or new experiment was launched.

### 2026-08-29 02:45 runtime checkpoint

- E5 now has 177/432 passing rows, or 354/864 encode/decode invocations; no
  failure/incomplete row exists.
- `x-ray` 128 KiB completed byte-exactly; the runner advanced to `xml` 32 KiB,
  the last Silesia file in this child.
- No experiment identity or parameter changed.

### 2026-08-29 02:50 runtime checkpoint

- E5 now has 178/432 passing rows, or 356/864 encode/decode invocations; no
  failure/incomplete row exists.
- `xml` 32 KiB completed byte-exactly and the same runner started `xml` 64 KiB.
- The experiment remains resumable under the original identity and parameters.

### 2026-08-29 02:58 runtime checkpoint

- E5 now has 179/432 passing rows, or 358/864 encode/decode invocations; no
  failure/incomplete row exists.
- `xml` 64 KiB completed byte-exactly; `xml` 128 KiB is now the active Auto
  encode case in `auto-b64-r1`.
- After this row, the runner will proceed to the next fixed Auto child; no
  duplicate experiment has been started.

### 2026-08-29 03:14 runtime checkpoint

- E5 now has 180/432 passing rows, or 360/864 encode/decode invocations; no
  failure/incomplete row exists.
- `auto-b64-r1` completed its full 36-row, 12-file child; the last `xml`
  128 KiB case passed byte-exactly.
- The parent is moving to the next fixed Auto block-size child under the same
  experiment identity.

### 2026-08-29 03:16 runtime checkpoint

- E5 now has 182/432 passing rows, or 364/864 encode/decode invocations; no
  failure/incomplete row exists.
- `auto-b64-r1` is complete at 36/36. The runner is in `auto-k2-b64-r1`,
  processing `dickens` 128 KiB after its 64 KiB row passed.
- No duplicate runner or parameter change occurred.

### 2026-08-29 04:00 runtime checkpoint

- E5 now has 213/432 passing rows, or 426/864 encode/decode invocations; no
  failure/incomplete row exists.
- `auto-k2-b64-r1` passed `x-ray` 128 KiB and is processing `xml` 32 KiB,
  the final file in this child.
- The stable experiment identity and executable remain unchanged.

### 2026-08-29 04:07 runtime checkpoint

- E5 now has 219/432 passing rows, or 438/864 encode/decode invocations; no
  failure/incomplete row exists.
- `auto-k4-b64-r1` completed all `dickens` scopes and is processing `mozilla`
  32 KiB.
- No duplicate runner or parameter change occurred.

### 2026-08-29 04:01 runtime checkpoint

- E5 now has 215/432 passing rows, or 430/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active `auto-k2-b64-r1` child passed `xml` 32/64 KiB and is encoding
  `xml` 128 KiB, its final row.
- No duplicate runner or parameter change occurred.

### 2026-08-29 03:46 runtime checkpoint

- E5 now has 205/432 passing rows, or 410/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active short-list child passed `sao` 32 KiB and is encoding `sao` 64 KiB.
- The stable experiment identity and executable remain unchanged.

### 2026-08-29 03:51 runtime checkpoint

- E5 now has 207/432 passing rows, or 414/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active short-list child passed `sao` 128 KiB and is processing `webster`
  32 KiB.
- No duplicate runner, code change, or parameter change occurred.

### 2026-08-29 04:10 runtime checkpoint

- E5 now has 221/432 passing rows, or 442/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active `auto-k4-b64-r1` child passed `mozilla` 32/64 KiB and is encoding
  `mozilla` 128 KiB.
- The stable experiment identity and executable remain unchanged.

### 2026-08-29 03:53 runtime checkpoint

- E5 now has 209/432 passing rows, or 418/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active `auto-k2-b64-r1` child passed `webster` 32/64 KiB and is encoding
  `webster` 128 KiB.
- The stable experiment identity and executable remain unchanged.

### 2026-08-29 03:47 runtime checkpoint

- E5 now has 206/432 passing rows, or 412/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active short-list child passed `sao` 64 KiB and is encoding `sao` 128 KiB.
- The stable experiment identity and executable remain unchanged.

### 2026-08-29 03:39 runtime checkpoint

- E5 now has 200/432 passing rows, or 400/864 encode/decode invocations; no
  failure/incomplete row exists.
- `auto-k2-b64-r1` passed `reymont` 64 KiB and is encoding `reymont` 128 KiB.
- The stable experiment identity and executable remain unchanged.

### 2026-08-29 03:45 runtime checkpoint

- E5 now has 204/432 passing rows, or 408/864 encode/decode invocations; no
  failure/incomplete row exists.
- `samba` 128 KiB completed byte-exactly and the active short-list child is
  processing `sao` 32 KiB.
- No duplicate runner or parameter change occurred.

### 2026-08-29 03:41 runtime checkpoint

- E5 now has 201/432 passing rows, or 402/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active short-list child passed `reymont` 128 KiB and is processing
  `samba` 32 KiB.
- No duplicate runner, code change, or parameter change occurred.

### 2026-08-29 03:43 runtime checkpoint

- E5 now has 203/432 passing rows, or 406/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active short-list child passed `samba` 32/64 KiB and is encoding
  `samba` 128 KiB.
- The stable experiment identity and executable remain unchanged.

### 2026-08-29 03:30 runtime checkpoint

- E5 now has 194/432 passing rows, or 388/864 encode/decode invocations; no
  failure/incomplete row exists.
- `auto-k2-b64-r1` is processing `ooffice` 128 KiB after the 32/64 KiB rows
  completed successfully.
- The experiment identity and executable remain unchanged.

### 2026-08-29 03:35 runtime checkpoint

- E5 now has 197/432 passing rows, or 394/864 encode/decode invocations; no
  failure/incomplete row exists.
- `auto-k2-b64-r1` is processing `osdb` 128 KiB after the 64 KiB case passed.
- The stable experiment identity and executable remain unchanged.

### 2026-08-29 03:38 runtime checkpoint

- E5 now has 199/432 passing rows, or 398/864 encode/decode invocations; no
  failure/incomplete row exists.
- `auto-k2-b64-r1` is encoding `reymont` 64 KiB after passing `reymont` 32 KiB
  and the preceding `osdb` scopes.
- No duplicate runner or parameter change occurred.

### 2026-08-29 03:19 runtime checkpoint

- E5 now has 183/432 passing rows, or 366/864 encode/decode invocations; no
  failure/incomplete row exists.
- The completed `auto-k2-b64-r1` rows cover `dickens` at 64/128 KiB, and the
  runner is processing `mozilla` 32 KiB.
- The stable runner and evidence identity remain unchanged.

### 2026-08-29 03:20 runtime checkpoint

- E5 now has 184/432 passing rows, or 368/864 encode/decode invocations; no
  failure/incomplete row exists.
- `auto-k2-b64-r1` is processing `mozilla` 64 KiB decode after its preceding
  case completed; the runner identity is unchanged.

### 2026-08-29 00:26 runtime checkpoint

- E5 reached 163/432 `COMPLETE` rows with `roundtrip=PASS`, or 326/864
  encode/decode invocations; no failed or incomplete row is present.
- The active child is still `auto-b64-r1` on `reymont` 64 KiB with a 64 KiB
  block. Its CPU activity is normal and the result row is not written yet.
- The final summary remains unavailable, so evidence derivation and README/Git
  publication remain gated.

### 2026-08-29 00:51 runtime checkpoint

- E5 now has 166/432 readable `COMPLETE` rows with `roundtrip=PASS`, or
  332/864 encode/decode invocations; no failure/incomplete row is present.
- `samba` 32 KiB completed under `auto-b64-r1`; the next active case is
  `samba` 64 KiB.
- One monitoring read briefly hit the runner's open `results.csv`; after retry,
  the count and PASS row were confirmed. No runner or codec restart occurred.

### 2026-08-29 01:00 runtime checkpoint

- E5 contains 167/432 passing rows, or 334/864 encode/decode invocations; no
  failure/incomplete row exists.
- The runner completed the `samba` 64 KiB case and started `samba` 128 KiB in
  `auto-b64-r1`.
- The experiment identity and executable are unchanged; final summary and
  post-E5 actions remain pending.

### 2026-08-29 01:18 runtime checkpoint

- E5 now has 168/432 passing rows, or 336/864 encode/decode invocations; no
  failure or incomplete row is present.
- The `samba` 128 KiB case completed successfully after the long Auto encode;
  the runner advanced to `sao` 32 KiB under `auto-b64-r1`.
- No duplicate process or parameter change occurred; final summary is still
  pending.

### 2026-08-29 00:31 runtime checkpoint

- E5 reached 164/432 passing case rows, or 328/864 encode/decode invocations;
  no failure or incomplete row is present.
- The runner completed `reymont` 64 KiB and started `reymont` 128 KiB under
  `auto-b64-r1`. The current codec process is CPU-active.
- No duplicate runner or new experiment was launched; final evidence remains
  gated on `summary.json`.

### 2026-08-29 00:46 runtime checkpoint

- E5 reached 165/432 passing rows, or 330/864 encode/decode invocations;
  failed and incomplete rows remain zero.
- The `reymont` 128 KiB case completed with 946.7288133 s encode and
  33.7774856 s decode, and the runner started `samba` 32 KiB.
- No duplicate runner or new experiment was launched. The final summary is
  still pending.

### 2026-08-29 01:23 runtime checkpoint

- E5 now contains 169/432 passing rows, or 338/864 encode/decode invocations;
  no failure/incomplete row exists.
- `sao` 32 KiB completed and the original runner started `sao` 64 KiB in
  `auto-b64-r1`.
- The experiment remains resumable under the same identity; final summary is
  not yet available.

### 2026-08-29 01:32 runtime checkpoint

- E5 now has 170/432 passing rows, or 340/864 encode/decode invocations; no
  failure/incomplete row is present.
- `sao` 64 KiB completed and `sao` 128 KiB is the active Auto encode case.
- Existing completed policy packages are reused; no duplicate matrix or
  parameter change occurred.

### 2026-08-29 01:52 runtime checkpoint

- E5 now has 171/432 passing rows, or 342/864 encode/decode invocations; no
  failure/incomplete row exists.
- The `sao` 128 KiB case completed with byte-exact decode. The original runner
  remains active and is moving to the next fixed Auto case.
- Final summary is still pending; no duplicate or replacement experiment was
  launched.

### 2026-08-29 01:57 runtime checkpoint

- E5 now has 172/432 passing rows, or 344/864 encode/decode invocations; no
  failure/incomplete row exists.
- `webster` 32 KiB completed byte-exactly and the same runner started the
  `webster` 64 KiB Auto case.
- The experiment identity and parameters remain unchanged.

# 2026-08-29 E5 closure and E6 acceleration

- E5 final package `results/experiments/hybridzip-r2-e5-router-320dd1b-v1`
  was validated read-only at `432/432` `COMPLETE/PASS` rows and `864/864`
  codec invocations. `summary.json` is present; no codec process remained.
- The first E6 Fast K=4 single-cell preflight was rejected by the runner
  validator, not by the codec: Fast recorded valid `fast-ext=1`, while the
  validator incorrectly required `zstd` for the policy name `fast`.
- `tools/run_silesia_experiment.ps1` now treats Fast as a policy and accepts
  only its defined block records `stored`, `fast-ext`, and `lz4`; forced modes
  and ratio shortlists keep their existing exact-mode checks. No archive format
  or codec implementation was changed.
- Resuming the retained preflight package
  `results/experiments/hybridzip-r2-e6-fast-k4-preflight-20260829` completed
  `4/4` timing rows and `8/8` codec invocations with exact input/decoded
  SHA-256. Its retained throughput was `0.6025 MB/s` encode and `0.6653 MB/s`
  decode for one 32 KiB `dickens` cell.
- Formal E6 package
  `results/experiments/hybridzip-r2-e6-fast-k4-full-20260829-w1` completed
  `432/432` rows (`108` warmup + `324` retained) across 12 Silesia files,
  32/64/128 KiB scopes, and 32/64/128 KiB internal blocks. All rows were
  byte-exact. The nine summary cells had minimum encode/decode throughput
  `0.5635/0.6112 MB/s`, both above the `0.16 MB/s` CPU floor. Fast worker
  count was `1`; ranker identity stayed
  `00010000|1025B343|4B1AC26C40AD4DA50312FD3B694D7E636FB768C2336FE773BC82D36424C27A4B`.
- Historical mode-2 E6 output remains separate evidence. It must not be
  merged into the current Fast K=4 claim. A second worker-count comparison is
  optional for F3 analysis and is not needed to establish the single-worker
  CPU floor.

# 2026-08-29 F1 artifact verification

- Existing training data is complete and file-level isolated: 9 training
  files and 3 validation files, derived from the complete 32 KiB forced
  oracle without codec runtime during feature export.
- Existing candidate fit is frozen at 2,644 canonical bytes, version
  `00010001`, CRC32 `A0354863`, SHA-256
  `CA1B144EF35E20EC388D739ACE9A1EF92A5E72410D050B5021C3A7F93C62D7B3`.
  Training and validation top-1 tied-winner recall are both `1.0` (`9/9`
  and `3/3`).
- `tools/test_fit_r2_fixed_point_ranker.py`,
  `tools/test_r2_ranker_training_set.ps1`, and
  `tools/test_r2_forced_oracle.ps1` all passed. These checks validate the
  artifact and no-leakage boundary, not K=8 shortlist archive regret.
- The production bootstrap model remains installed. Promoting the candidate
  still requires a separately recorded E5 run with the candidate model and
  held-out archive-byte regret evidence.

# 2026-08-29 final R2 scoped verification

- Completed-package resume check for E5 returned `Experiment already complete`
  without starting codec work.
- Full Release CTest passed `18/18` in `47.21 s`, including HZ01 core and
  pipeline tests, donor backends, R2 codec, and structure routing.
- The scoped R2 implementation objective is evidenced by current artifacts:
  same-input manifest/provenance, current E5 K=8 router matrix, current E6
  Fast K=4 matrix, Fast block executor smoke, and byte-exact HZ01 regression.
- Remaining research items are explicitly outside this scoped completion:
  promoted held-out ranker weights, complete-file Silesia ratio acceptance,
  Tencent/OASum materialization, and GPU throughput.

# 2026-08-29 10:42: E5 runtime checkpoint

- E5 now contains `320/432` durable `COMPLETE/PASS` rows (`640/864`
  encode/decode invocations), with zero failure/error rows.
- `x-ray / 64 KiB / Auto / 128 KiB` passed byte-exactly; the runner advanced
  to `x-ray / 128 KiB / Auto / 128 KiB`.
- The experiment identity and parameters remain unchanged.

# 2026-08-29 10:50: E5 runtime checkpoint

- E5 now contains `321/432` durable `COMPLETE/PASS` rows (`642/864`
  encode/decode invocations), with zero failure/error rows.
- `x-ray / 128 KiB / Auto / 128 KiB` completed encode and byte-exact decode.
- The original runner advanced to `xml / 32 KiB / Auto / 128 KiB`, the first
  row of the final Silesia file in `auto-b128-r1`.

# 2026-08-29 10:57: E5 runtime checkpoint

- E5 now contains `323/432` durable `COMPLETE/PASS` rows (`646/864`
  encode/decode invocations), with zero failure/error rows.
- `xml / 64 KiB / Auto / 128 KiB` passed byte-exactly; the final
  `auto-b128-r1` row `xml / 128 KiB` is now encoding.

# 2026-08-29 11:07: E5 runtime checkpoint

- E5 now contains `324/432` durable `COMPLETE/PASS` rows (`648/864`
  encode/decode invocations), with zero failure/error rows.
- The final `auto-b128-r1` row (`xml / 128 KiB`) passed byte-exactly and that
  child package is complete.
- The same runner advanced to `auto-k2-b128-r1`, currently decoding
  `dickens / 32 KiB`; no experiment identity or parameter changed.

# 2026-08-29 11:23: E5 runtime checkpoint

- E5 now contains `338/432` durable `COMPLETE/PASS` rows (`676/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k2-b128-r1` completed through `nci / 128 KiB`; the active case is
  `ooffice / 128 KiB / auto-k2 / 128 KiB`.
- The original runner and experiment identities remain unchanged.

# 2026-08-29 11:55: E5 runtime checkpoint

- E5 now contains `360/432` durable `COMPLETE/PASS` rows (`720/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k2-b128-r1` completed all 36 cases byte-exactly.
- The original runner advanced to `auto-k4-b128-r1`, currently encoding
  `dickens / 32 KiB / auto-k4 / 128 KiB`.

# 2026-08-29 12:22: E5 runtime checkpoint

- E5 now contains `374/432` durable `COMPLETE/PASS` rows (`748/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `ooffice / 64 KiB`; the active case is
  `ooffice / 128 KiB / auto-k4 / 128 KiB`.

# 2026-08-29 12:27: E5 runtime checkpoint

- E5 now contains `376/432` durable `COMPLETE/PASS` rows (`752/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `ooffice / 128 KiB` and `osdb / 32 KiB`;
  the active case is `osdb / 64 KiB / auto-k4 / 128 KiB` decode.

# 2026-08-29 12:36: E5 runtime checkpoint

- E5 now contains `380/432` durable `COMPLETE/PASS` rows (`760/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed through `reymont / 64 KiB`; the active case is
  `reymont / 128 KiB / auto-k4 / 128 KiB` decode.

# 2026-08-29 12:41: E5 runtime checkpoint

- E5 now contains `383/432` durable `COMPLETE/PASS` rows (`766/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `reymont / 128 KiB`; the active case is
  `samba / 128 KiB / auto-k4 / 128 KiB`.

# 2026-08-29 12:46: E5 runtime checkpoint

- E5 now contains `384/432` durable `COMPLETE/PASS` rows (`768/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `samba / 128 KiB`; the active case is
  `sao / 32 KiB / auto-k4 / 128 KiB` decode.

# 2026-08-29 12:51: E5 runtime checkpoint

- E5 now contains `386/432` durable `COMPLETE/PASS` rows (`772/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `ooffice / 32,64 KiB`; the active case is
  `sao / 128 KiB / auto-k4 / 128 KiB`.

# 2026-08-29 12:55: E5 runtime checkpoint

- E5 now contains `388/432` durable `COMPLETE/PASS` rows (`776/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `sao / 128 KiB` and `webster / 32 KiB`; the
  active case is `webster / 64 KiB / auto-k4 / 128 KiB` decode.

# 2026-08-29 13:00: E5 runtime checkpoint

- E5 now contains `391/432` durable `COMPLETE/PASS` rows (`782/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `webster / 64,128 KiB` and `x-ray / 32 KiB`;
  the active case is `x-ray / 64 KiB / auto-k4 / 128 KiB`.

# 2026-08-29 13:05: E5 runtime checkpoint

- E5 now contains `392/432` durable `COMPLETE/PASS` rows (`784/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `x-ray / 64 KiB`; the active case is
  `x-ray / 128 KiB / auto-k4 / 128 KiB` decode.

# 2026-08-29 13:09: E5 runtime checkpoint

- E5 now contains `395/432` durable `COMPLETE/PASS` rows (`790/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `x-ray / 128 KiB` and `xml / 32,64 KiB`; the
  active case is its final `xml / 128 KiB` row.

# 2026-08-29 13:15: E5 runtime checkpoint

- E5 now contains `398/432` durable `COMPLETE/PASS` rows (`796/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed all 36 cases byte-exactly.
- The original runner advanced to the final `auto-k8-b128-r1` child,
  currently encoding `dickens / 128 KiB / auto-k8 / 128 KiB`.

# 2026-08-29 13:30: E5 runtime checkpoint

- E5 now contains `406/432` durable `COMPLETE/PASS` rows (`812/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k8-b128-r1` completed `mr / 128 KiB` and `nci / 32 KiB`; the active
  case is `nci / 64 KiB / auto-k8 / 128 KiB` decode.

# 2026-08-29 13:35: E5 runtime checkpoint

- E5 now contains `409/432` durable `COMPLETE/PASS` rows (`818/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k8-b128-r1` completed `nci / 64,128 KiB` and `ooffice / 32 KiB`;
  the active case is `ooffice / 64 KiB / auto-k8 / 128 KiB` decode.

# 2026-08-29 13:52: E5 runtime checkpoint

- E5 now contains `417/432` durable `COMPLETE/PASS` rows (`834/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k8-b128-r1` completed through `reymont / 128 KiB`; the active case is
  `samba / 32 KiB / auto-k8 / 128 KiB`.

# 2026-08-29 14:13: E5 runtime checkpoint

- E5 now contains `426/432` durable `COMPLETE/PASS` rows (`852/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k8-b128-r1` completed `sao / 128 KiB` and all `webster` scopes; the
  active case is `x-ray / 32 KiB / auto-k8 / 128 KiB`.

# 2026-08-29 11:59: E5 runtime checkpoint

- E5 now contains `362/432` durable `COMPLETE/PASS` rows (`724/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `dickens / 32,64 KiB`; the active case is
  `dickens / 128 KiB / auto-k4 / 128 KiB`.

# 2026-08-29 12:04: E5 runtime checkpoint

- E5 now contains `365/432` durable `COMPLETE/PASS` rows (`730/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `dickens / 128 KiB`; the active case is
  `mozilla / 128 KiB / auto-k4 / 128 KiB`.

# 2026-08-29 12:08: E5 runtime checkpoint

- E5 now contains `367/432` durable `COMPLETE/PASS` rows (`734/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` completed `mozilla / 128 KiB` and `mr / 32 KiB`; the
  active case is `mr / 64 KiB / auto-k4 / 128 KiB`.

# 2026-08-29 12:17: E5 runtime checkpoint

- E5 now contains `372/432` durable `COMPLETE/PASS` rows (`744/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k4-b128-r1` has completed through `nci / 128 KiB`; the active case is
  `ooffice / 32 KiB / auto-k4 / 128 KiB` decode.

# 2026-08-29 11:48: E5 runtime checkpoint

- E5 now contains `355/432` durable `COMPLETE/PASS` rows (`710/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k2-b128-r1` completed `webster / 128 KiB` and `x-ray / 32 KiB`; the
  active case is `x-ray / 64 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity remain unchanged.

# 2026-08-29 11:27: E5 runtime checkpoint

- E5 now contains `341/432` durable `COMPLETE/PASS` rows (`682/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k2-b128-r1` completed `ooffice / 128 KiB` and `osdb / 32,64 KiB`;
  the active case is `osdb / 128 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity are unchanged.

# 2026-08-29 11:31: E5 runtime checkpoint

- E5 now contains `344/432` durable `COMPLETE/PASS` rows (`688/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k2-b128-r1` completed `osdb / 128 KiB`; the active case is
  `reymont / 128 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity remain unchanged.

# 2026-08-29 11:44: E5 runtime checkpoint

- E5 now contains `352/432` durable `COMPLETE/PASS` rows (`704/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k2-b128-r1` completed `sao / 128 KiB`; the active case is
  `webster / 64 KiB / auto-k2 / 128 KiB` decode.
- The runner, executable, manifest, and experiment identity remain unchanged.

# 2026-08-29 11:39: E5 runtime checkpoint

- E5 now contains `350/432` durable `COMPLETE/PASS` rows (`700/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k2-b128-r1` completed `samba / 128 KiB`; the active case is
  `sao / 128 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity remain unchanged.

# 2026-08-29 11:34: E5 runtime checkpoint

- E5 now contains `347/432` durable `COMPLETE/PASS` rows (`694/864`
  encode/decode invocations), with zero failure/error rows.
- `auto-k2-b128-r1` completed `reymont / 128 KiB`; the active case is
  `samba / 128 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity remain unchanged.

### 2026-08-29 02:32 runtime checkpoint

- E5 now has 176/432 passing rows, or 352/864 encode/decode invocations; no
  failure/incomplete row exists.
- `x-ray` 64 KiB completed byte-exactly and `x-ray` 128 KiB is the active Auto
  case.
- No duplicate runner or new experiment was launched.

### 2026-08-29 02:21 runtime checkpoint

- E5 now has 174/432 passing rows, or 348/864 encode/decode invocations; no
  failure/incomplete row exists.
- `webster` 128 KiB completed with byte-exact decode; the runner started
  `x-ray` 32 KiB under `auto-b64-r1`.
- The final summary remains pending and the experiment identity is unchanged.

### 2026-08-29 02:25 runtime checkpoint

- E5 now has 175/432 passing rows, or 350/864 encode/decode invocations; no
  failure/incomplete row exists.
- `x-ray` 32 KiB completed byte-exactly; the original runner is advancing to
  the next Auto case.
- The experiment identity and parameters remain unchanged.

### 2026-08-29 02:05 runtime checkpoint

- E5 now has 173/432 passing rows, or 346/864 encode/decode invocations; no
  failure or incomplete row exists.
- `webster` 64 KiB completed with byte-exact decode; the original runner is
  moving to the next fixed Auto case.
- The final summary is still pending and no replacement run was launched.

### 2026-08-29 03:22 runtime checkpoint

- E5 now has 187/432 passing rows, or 374/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active `auto-k2-b64-r1` child passed `mr` 32 KiB and is encoding `mr`
  64 KiB; previous `mozilla` scopes are complete.
- The stable experiment identity and executable remain unchanged.

### 2026-08-29 03:25 runtime checkpoint

- E5 now has 189/432 passing rows, or 378/864 encode/decode invocations; no
  failure/incomplete row exists.
- The active short-list child completed `mr` 128 KiB and is processing `nci`
  32 KiB under `auto-k2-b64-r1`.
- No duplicate runner or parameter change occurred.

# Runtime checkpoint 2026-08-29 04:18 +08:00

- E5 matrix `hybridzip-r2-e5-router-320dd1b-v1`: 226/432 rows complete.
- All recorded rows are byte-exact `COMPLETE/PASS`; failures: 0.
- One runner remains active (PID 30912); current codec child is `nci` 64 KiB
  encode under `auto-k4-b64-r1`.
- Final `summary.json` is still pending; no duplicate run was started.
# 2026-08-29 06:48: E5 direct recovery checkpoint

- Direct inspection of `results/experiments/hybridzip-r2-e5-router-320dd1b-v1`
  found `291/432` result rows, all `PASS`, with no failure/error row.
- PID `30912` remains the sole parent runner and one `hybridzip.exe` child is
  CPU-active in the final `auto-b128-r1` block-size group.
- `summary.json` has not been emitted. Keep the same resumable experiment and
  defer documentation, commits, E6, and HZ03 work until E5 completion.

# 2026-08-29 08:31: E5 partial ledger validation

- Read-only validation covered `304` recorded E5 rows in
  `results/experiments/hybridzip-r2-e5-router-320dd1b-v1`.
- Every row was `COMPLETE/PASS`; all input, archive, and decoded files were
  present with matching recorded lengths and SHA-256 values.
- Every input SHA-256 equaled its decoded SHA-256; malformed/error count was
  zero. This is an interim checkpoint, not final `432`-row evidence.

# 2026-08-29 08:34: E5 partial shortlist derivation

- Read-only forced-oracle derivation completed at
  `results/analysis/r2-forced-oracle-derived-320dd1b-e5-partial-20260829`.
- The current E5 package matched `36` rows (12 files at the 32 KiB scope) and
  left `180` rows uncovered because the forced ledger is 32 KiB only.
- Tie-aware shortlist results on the matched rows: `auto-k2` recall `7/12`
  (`58.3333%`) with `291` aggregate regret bytes; `auto-k4` recall `12/12`
  (`100%`) with zero regret; `auto-k8` recall `12/12` (`100%`) with zero
  regret.
- These are partial evidence only; final R2 claims require the complete E5
  package and its generated summary.

# 2026-08-29 08:37: focused R2 regression

- Ran `ctest --test-dir build -C Release --output-on-failure -R
  "structure_routing_tests|r2_codec_tests"` against the existing Release
  build while E5 remained active.
- `hz_r2_codec_tests` and `hz_structure_routing_tests` both passed (`2/2`);
  total test time was `41.79 s`.
- No source rebuild, experiment restart, or parameter change occurred.

# 2026-08-29 08:44: HZ01 compatibility regression

- Ran the existing Release-build tests `hz_core_tests` and
  `hz_pipeline_tests` without rebuilding or touching the active E5 package.
- Both tests passed (`2/2`) in `0.64 s`, covering the core HZ01 contract and
  the legacy four-expert pipeline lifecycle.

# 2026-08-29 09:46: E5 runtime checkpoint

- Current E5 package `results/experiments/hybridzip-r2-e5-router-320dd1b-v1`
  contains `312/432` durable `COMPLETE/PASS` rows (`624/864` codec
  invocations), with zero failure/error rows.
- PID `30912` remains the only matrix runner. Its only codec child is running
  `sao` at the 32 KiB scope with Auto policy and a 128 KiB internal block.
- `summary.json` has not been emitted. No source, executable, experiment
  identity, or parameter was changed during this checkpoint.

# 2026-08-29: External-Core Kill Test Decision Notes

## Confirmed Current Boundary

- Current E5 Auto/K4/K8 evidence is complete and establishes that router
  shortlist behavior is not the dominant high-ratio bottleneck on the frozen
  Silesia matrix. It does not establish generalization beyond those inputs.
- Current Fast K=4 E6 evidence exceeds the CPU floor, but its measured bpb is
  materially above PAQ8px v216 `-1` on the same frozen leading prefixes.
- The historical 32 KiB forced-oracle ledger has only `paq8px-generic-sse`
  and `paq8px-detected-sse` winners. This cannot be generalized to full files,
  larger superblocks, or Tencent/OASum.

## Decision

Stop feature expansion temporarily. The next information-bearing experiment
is an external-core kill test, not additional R2 routing work.

## Candidate Set

1. Full upstream Kanzi C++ at high compression levels.
2. Full libbsc.
3. Official PAQ8px v216 at `-1/-2/-3/-4`.
4. XZ `-9e` control.
5. Existing HybridZip Auto and Fast controls.

## Execution Discipline

- Every row must use identical input SHA-256 and include complete archive
  bytes, encode/decode time, peak memory, command, tool identity, and decoded
  SHA-256.
- Begin with a 1 KiB byte-exact smoke, then the frozen 32/64/128 KiB inputs.
- Do not launch the full 12-file superblock grid until a candidate passes the
  small-block screen. PAQ `-2/-3/-4` must be staged because their runtime may
  be large.
- The initial small-block results decide only whether a candidate advances;
  they do not decide the final architecture.

## Source Inventory Finding

- `E:\MIXER\KU\hybridzip-r2` already contains the recorded Kanzi source,
  PAQ8px source, and XZ source. They must be identity-checked and reused.
- No standalone libbsc root is currently present in `E:\MIXER\KU`; acquisition
  needs a new non-overwriting provenance directory.

# 2026-08-29 09:53: E5 runtime checkpoint

- E5 now contains `314/432` durable `COMPLETE/PASS` rows (`628/864`
  encode/decode invocations), with zero failure/error rows.
- The original runner PID `30912` remains active and has started the
  `sao / 128 KiB / Auto / 128 KiB` case after the preceding 64 KiB row passed.
- No source, executable, experiment identity, or matrix parameter changed;
  `summary.json` remains absent until all child packages finish.

# 2026-08-29 10:08: E5 runtime checkpoint

- E5 now contains `315/432` durable `COMPLETE/PASS` rows (`630/864`
  encode/decode invocations), with zero failure/error rows.
- The `sao / 128 KiB / Auto / 128 KiB` case passed byte-exact decode and the
  original runner continued to the next case without a restart.
- No source, executable, experiment identity, or matrix parameter changed;
  `summary.json` remains pending.

# 2026-08-29 10:10: E5 runtime checkpoint

- E5 now contains `316/432` durable `COMPLETE/PASS` rows (`632/864`
  encode/decode invocations), with zero failure/error rows.
- The original runner advanced to `webster / 64 KiB / Auto / 128 KiB` under
  the unchanged `auto-b128-r1` child package.
- No source, executable, experiment identity, or matrix parameter changed;
  `summary.json` remains absent.

# 2026-08-29 10:18: E5 runtime checkpoint

- E5 now contains `317/432` durable `COMPLETE/PASS` rows (`634/864`
  encode/decode invocations), with zero failure/error rows.
- `webster / 64 KiB / Auto / 128 KiB` passed byte-exactly; the original runner
  advanced to `webster / 128 KiB / Auto / 128 KiB` in the same child package.
- No source, executable, experiment identity, or matrix parameter changed;
  `summary.json` remains pending.

# 2026-08-29 10:26: E5 runtime checkpoint

- E5 remains at `317/432` durable `COMPLETE/PASS` rows with zero
  failure/error rows.
- The same codec child is still processing `webster / 128 KiB / Auto / 128 KiB`;
  CPU time is increasing and the process is responsive.
- No duplicate runner, source change, executable change, or parameter change
  occurred; `summary.json` remains absent.

# 2026-08-29 10:29: E5 runtime checkpoint

- E5 remains at `317/432` durable `COMPLETE/PASS` rows with zero
  failure/error rows.
- `webster / 128 KiB / Auto / 128 KiB` remains active; its codec process is
  responsive and CPU time continues to increase.
- The single runner and experiment identity are preserved; final validation
  remains gated on package completion.

# 2026-08-29 10:34: E5 runtime checkpoint

- E5 now contains `318/432` durable `COMPLETE/PASS` rows (`636/864`
  encode/decode invocations), with zero failure/error rows.
- `webster / 128 KiB / Auto / 128 KiB` completed encode and byte-exact decode.
- The same runner advanced to `x-ray / 32 KiB / Auto / 128 KiB`; experiment
  identity and parameters are unchanged.

# 2026-08-29 10:38: E5 runtime checkpoint

- E5 now contains `319/432` durable `COMPLETE/PASS` rows (`638/864`
  encode/decode invocations), with zero failure/error rows.
- `x-ray / 32 KiB / Auto / 128 KiB` passed byte-exactly; the runner advanced
  to `x-ray / 64 KiB / Auto / 128 KiB`.
- The experiment identity and parameters remain unchanged.
