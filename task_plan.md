# HybridZip R2 Continuation Plan

## Runtime progress checkpoint: E5 active (2026-08-29 09:30 +08:00)

- `samba` 64 KiB Auto completed with byte-exact decode; E5 reached
  `311/432` durable `COMPLETE/PASS` rows with zero failures.
- The same runner PID `30912` advanced to `samba` 128 KiB Auto under
  `auto-b128-r1`; no duplicate runner or parameter change occurred.
- `summary.json` remains absent. Preserve the package and defer final E5
  derivation and E6 runtime until all `432` rows complete.

## Runtime progress checkpoint: E5 active (2026-08-29 09:25 +08:00)

- `samba` 32 KiB Auto completed with byte-exact decode; E5 reached
  `310/432` durable `COMPLETE/PASS` rows with zero failures.
- The same runner PID `30912` advanced to `samba` 64 KiB Auto under
  `auto-b128-r1`; no duplicate runner or parameter change occurred.
- `summary.json` remains absent. Preserve the package and defer final E5
  derivation and E6 runtime until all `432` rows complete.

## Runtime progress checkpoint: E5 active (2026-08-29 09:22 +08:00)

- `reymont` 128 KiB Auto completed with byte-exact decode; E5 reached
  `309/432` durable `COMPLETE/PASS` rows with zero failures.
- The same runner PID `30912` advanced to `samba` 32 KiB Auto under
  `auto-b128-r1`; no duplicate runner or parameter change occurred.
- `summary.json` remains absent. Preserve the package and defer final E5
  derivation and E6 runtime until all `432` rows complete.

## Runtime progress checkpoint: E5 active (2026-08-29 09:17 +08:00)

- E5 remains at `308/432` durable `COMPLETE/PASS` rows with zero failures;
  `reymont` 32/64 KiB rows and all earlier groups remain byte-exact.
- The sole runner PID `30912` has kept codec PID `13916` on
  `reymont` 128 KiB Auto under `auto-b128-r1`; CPU time continues increasing
  and the case has not restarted.
- `summary.json` is absent. Preserve the current package and defer final
  derivation and E6 runtime until the remaining `124` rows complete.

## Runtime progress checkpoint: E5 active (2026-08-29 09:09 +08:00)

- `reymont` 64 KiB Auto completed with byte-exact decode; E5 reached
  `308/432` durable `COMPLETE/PASS` rows with zero failures.
- The same runner PID `30912` advanced to `reymont` 128 KiB Auto under
  `auto-b128-r1`; no duplicate runner or parameter change occurred.
- `summary.json` remains absent. Preserve the package and defer final E5
  derivation and E6 runtime until all `432` rows complete.

## Runtime progress checkpoint: E5 active (2026-08-29 09:01 +08:00)

- `reymont` 32 KiB Auto completed with byte-exact decode; E5 is now
  `307/432` durable `COMPLETE/PASS` rows with zero failures.
- The same runner PID `30912` advanced to `reymont` 64 KiB Auto under
  `auto-b128-r1`; no duplicate runner or parameter change occurred.
- `summary.json` remains absent. Preserve the package and defer final E5
  derivation and E6 runtime until all `432` rows complete.

## Runtime progress checkpoint: E5 active (2026-08-29 08:57 +08:00)

- `osdb` 128 KiB Auto completed with byte-exact decode; E5 reached
  `306/432` durable `COMPLETE/PASS` rows with zero failures.
- The same runner PID `30912` advanced to `reymont` 32 KiB Auto under the
  final `auto-b128-r1` child; no duplicate runner or parameter change occurred.
- `summary.json` is still absent. Preserve the package and defer final E5
  derivation and E6 runtime until all `432` rows complete.

## Runtime progress checkpoint: E5 active (2026-08-29 08:44 +08:00)

- E5 remains at `305/432` durable `COMPLETE/PASS` rows with zero failures.
- The sole runner PID `30912` and codec PID `6872` are still active on
  `osdb` 128 KiB Auto with a 128 KiB block; CPU time continues increasing.
- `summary.json` is absent. Preserve the current package; do not start E6 or
  a replacement runner until E5 reaches all `432` rows.

## Runtime progress checkpoint: E5 active (2026-08-29 08:38 +08:00)

- E5 reached `305/432` durable `COMPLETE/PASS` rows with zero failures;
  `osdb` 64 KiB completed with byte-exact decode.
- The same runner PID `30912` advanced to `osdb` 128 KiB Auto encoding under
  `auto-b128-r1`; no duplicate runner or experiment was started.
- Focused R2 regression (`hz_r2_codec_tests` and
  `hz_structure_routing_tests`) passed `2/2` against the existing Release
  build.
- `summary.json` remains absent; preserve the package and defer final E5
  derivation and E6 runtime until completion.

## Runtime progress checkpoint: E5 active (2026-08-29 08:28 +08:00)

- E5 is at `304/432` durable `COMPLETE/PASS` rows with zero failures.
- The same runner PID `30912` is executing `osdb` 64 KiB Auto under
  `auto-b128-r1`; codec PID `18252` remains CPU-active and has not restarted.
- `summary.json` is absent. Preserve the experiment package and defer E5
  derivation, docs, commits, and E6 runtime until completion.

## Runtime progress checkpoint: E5 active (2026-08-29 08:22 +08:00)

- `ooffice` 128 KiB Auto completed with byte-exact decode; E5 is now
  `303/432` durable `COMPLETE/PASS` rows with zero failures.
- The same runner PID `30912` advanced to `osdb` 32 KiB Auto under the final
  `auto-b128-r1` child; no duplicate process or parameter change occurred.
- `summary.json` remains absent. Keep the package intact and defer derivation,
  documentation, commits, and E6 until all `432` rows complete.

## Runtime progress checkpoint: E5 active (2026-08-29 08:18 +08:00)

- The single E5 runner (PID `30912`) remains active for experiment
  `hybridzip-r2-e5-router-320dd1b-v1`; no duplicate runner was started.
- Durable case rows remain `302/432`, all `COMPLETE/PASS`, with zero failures.
- Codec PID `6844` is still CPU-active on `ooffice` 128 KiB, Auto policy,
  128 KiB block size; its CPU time is increasing and the output is not yet
  complete.
- `summary.json` is absent. Preserve the current package and defer E5
  derivation, documentation, commits, and E6 runtime until completion.

## Runtime progress checkpoint: E5 active (2026-08-29 08:11 +08:00)

- E5 remains at `302/432` durable `COMPLETE/PASS` rows with zero failures.
- Codec PID `6844` is responsive and CPU-active on `ooffice` 128 KiB Auto
  encoding under `auto-b128-r1`.
- Preserve the same runner; `summary.json` is still absent.

## Runtime progress checkpoint: E5 active (2026-08-29 08:11 +08:00)

- `ooffice` 64 KiB completed with byte-exact `PASS`; E5 reached `302/432`.
- The sole runner advanced to `ooffice` 128 KiB Auto encoding under
  `auto-b128-r1`; no failure or duplicate runner was observed.

## Runtime progress checkpoint: E5 active (2026-08-29 08:08 +08:00)

- E5 remains at `301/432` durable `COMPLETE/PASS` rows with zero failures.
- PID `17452` is responsive and CPU-active on `ooffice` 64 KiB Auto encoding
  under `auto-b128-r1` (about 4.5 minutes elapsed).
- Keep the existing runner and package; `summary.json` is still absent.

## Runtime progress checkpoint: E5 active (2026-08-29 08:05 +08:00)

- E5 remains at `301/432` durable `COMPLETE/PASS` rows with zero failures.
- Codec PID `17452` is responsive and CPU-active on `ooffice` 64 KiB Auto
  encoding under `auto-b128-r1`.
- Preserve the same runner and experiment identity; no summary exists yet.

## Runtime progress checkpoint: E5 active (2026-08-29 08:02 +08:00)

- `nci` 128 KiB and `ooffice` 32 KiB completed byte-exactly; E5 reached
  `301/432` durable `PASS` rows.
- The sole runner advanced to `ooffice` 64 KiB under `auto-b128-r1`; no
  failure or duplicate runner was observed.

## Runtime progress checkpoint: E5 active (2026-08-29 08:00 +08:00)

- `nci` 128 KiB completed with byte-exact `PASS`; E5 reached `300/432` rows.
- The sole runner advanced to `ooffice` 32 KiB under `auto-b128-r1`; no
  failure or duplicate runner was observed.

## Runtime progress checkpoint: E5 active (2026-08-29 07:57 +08:00)

- E5 remains at `299/432` durable rows, all `COMPLETE/PASS`, with zero
  failures.
- Codec PID `30012` has run continuously on `nci` 128 KiB Auto encoding since
  07:44 and remains responsive with increasing CPU time.
- The protected archive remains temporary until atomic completion; do not kill
  or restart the runner.

## Runtime progress checkpoint: E5 active (2026-08-29 07:55 +08:00)

- E5 remains at `299/432` durable `COMPLETE/PASS` rows with zero failures.
- `nci` 128 KiB Auto encoding is still active in one responsive codec thread;
  its `.tmp` archive remains zero bytes until atomic encode completion.
- Preserve the runner and do not infer a failure from the unchanged row count.

## Runtime progress checkpoint: E5 active (2026-08-29 07:53 +08:00)

- E5 remains at `299/432` durable `COMPLETE/PASS` rows; zero failure rows.
- Codec PID `30012` has been active since 07:44 on `nci` 128 KiB Auto encoding,
  remains responsive, and has not been restarted.
- `summary.json` is absent; preserve the current package and runner.

## Runtime progress checkpoint: E5 active (2026-08-29 07:52 +08:00)

- E5 remains at `299/432` durable rows, all `COMPLETE/PASS`, with zero
  non-pass rows.
- Codec PID `30012` is responsive and CPU-active on `nci` 128 KiB under
  `auto-b128-r1`; no duplicate runner is present.
- `summary.json` is absent. Preserve the current experiment and defer E5
  derivation and E6 runtime until completion.

## Runtime progress checkpoint: E5 active (2026-08-29 07:50 +08:00)

- Direct inspection shows `299/432` durable E5 rows, all `COMPLETE/PASS`;
  `auto-b128-r1` has `11/36` rows complete.
- The current codec PID `30012` is responsive and CPU-active on `nci` 128 KiB.
- No `summary.json` exists. Keep the same runner and defer E5 analysis and E6
  runtime until the matrix completes.

## Runtime progress checkpoint: E5 active (2026-08-29 07:45 +08:00)

- `nci` 64 KiB completed with byte-exact `PASS`; E5 reached `299/432` rows.
- The sole runner advanced to `nci` 128 KiB under `auto-b128-r1` with no
  failure or duplicate runner.

## Runtime progress checkpoint: E5 active (2026-08-29 07:39 +08:00)

- `nci` 32 KiB completed with byte-exact `PASS`; E5 remains `298/432` while
  `nci` 64 KiB is being encoded.
- Codec PID `7112` is responsive and CPU-active under the sole runner PID
  `30912`; no failure or duplicate run exists.

## Runtime progress checkpoint: E5 active (2026-08-29 07:37 +08:00)

- `nci` 32 KiB completed with byte-exact `PASS`; E5 reached `298/432` rows.
- The sole runner advanced to `nci` 64 KiB under `auto-b128-r1`; no failure or
  duplicate runner was observed.

## Runtime progress checkpoint: E5 active (2026-08-29 07:34 +08:00)

- `mr` 128 KiB completed encoding and byte-exact decoding; E5 reached
  `297/432` durable `PASS` rows.
- The sole runner advanced to `nci` 32 KiB under `auto-b128-r1`; no failure or
  duplicate runner was observed.

## Runtime progress checkpoint: E5 active (2026-08-29 07:21 +08:00)

- E5 remains at `296/432` durable `PASS` rows with zero failures.
- The active codec PID `19888` is encoding `mr` 128 KiB under `auto-b128-r1`;
  it is responsive and CPU-active.
- No summary has been generated and no duplicate runner is present.

## Runtime progress checkpoint: E5 active (2026-08-29 07:19 +08:00)

- `mr` 64 KiB completed with byte-exact `PASS`; E5 reached `296/432` rows.
- The active child advanced to `mr` 128 KiB. `auto-b128-r1` has 8/36 durable
  rows; the remaining child packages have not started.

## Runtime progress checkpoint: E5 active (2026-08-29 07:13 +08:00)

- E5 advanced to `295/432` durable rows; all recorded rows remain
  `COMPLETE/PASS` with zero failures.
- PID `30912` remains the sole parent runner and has advanced to the next
  fixed case under `auto-b128-r1`.

## Runtime progress checkpoint: E5 active (2026-08-29 07:11 +08:00)

- `mr` 32 KiB completed with byte-exact `PASS`; E5 is now `294/432` rows.
- The sole runner advanced within `auto-b128-r1`; no failure or duplicate
  runner was observed.

## Runtime progress checkpoint: E5 active (2026-08-29 07:10 +08:00)

- `mozilla` 128 KiB completed in `auto-b128-r1` with a 11,547-byte archive;
  the runner advanced to `mr` 32 KiB.
- The durable row count is still `293/432` until the new case finishes its
  byte-exact decode; all recorded rows remain `PASS`.
- PID `30912` remains the sole runner. Preserve the same experiment identity.

## Benchmark provenance audit (2026-08-29 07:05 +08:00)

- `bench/manifests/silesia-leading-32-64-128.tsv` validates at 36 rows,
  12 files, scopes 32/64/128 KiB, with zero missing source paths; its SHA-256
  remains `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D`.
- The same-input PAQ8px v216 `-1` package has 36/36 `PASS` rows.
- Dataset distribution boundaries and the staged zstd donor provenance are
  recorded in `docs/DATASET_PROVENANCE.md` and `docs/provenance/zstd-v1.5.7.json`.

## Partial evidence checkpoint (2026-08-29 07:03 +08:00)

- Read-only validation of the 293 recorded rows found zero malformed rows:
  every row is `COMPLETE/PASS`, has equal input/decoded SHA-256, and a positive
  archive length.
- The partial rows cover all four policies and all three scopes; their summed
  archive bytes and timings are intentionally not treated as final metrics.

## Runtime progress checkpoint: E5 active (2026-08-29 07:00 +08:00)

- Direct inspection shows `293/432` durable E5 rows, all `PASS`, with zero
  non-pass rows.
- PID `30912` remains the only parent runner; codec child PID `18468` is
  CPU-active on `auto-b128-r1 / mozilla / 128 KiB`.
- `summary.json` is absent; keep the existing experiment identity and do not
  launch a replacement or post-E5 runtime.

## Runtime progress checkpoint: E5 active (2026-08-29 06:58 +08:00)

- E5 remains at `292/432` durable rows, all `PASS`; the previous `mozilla`
  64 KiB case has completed and the runner advanced to `mozilla` 128 KiB.
- PID `30912` is still the sole parent runner; codec child PID `16800` is
  executing the same `auto-b128-r1` group.
- `summary.json` is absent. Preserve the current package and resume identity.

## Runtime progress checkpoint: E5 active (2026-08-29 06:56 +08:00)

- Direct inspection remains at `292/432` durable E5 case rows; all are `PASS`.
- PID `30912` is the only parent runner and codec child PID `14224` is still
  CPU-active on the `auto-b128-r1` group (`mozilla` 64 KiB case).
- `summary.json` is absent. The same experiment ID and build must be resumed;
  no duplicate runner or post-E5 workload is authorized.

## Runtime progress checkpoint: E5 active (2026-08-29 06:48 +08:00)

- Direct inspection of the active package shows `291/432` durable E5 case rows,
  all `PASS`, with zero non-pass rows.
- The single authorized parent runner is PID `30912`; one `hybridzip.exe`
  child is active in the current `auto-b128-r1` group.
- `summary.json` is still absent. Preserve the experiment ID, executable, and
  matrix; do not start a duplicate runner or post-E5 workload.

## E6 preflight checkpoint (2026-08-29 06:53 +08:00)

- Read-only `run_r2_e5_e6_matrix.ps1 -Stage e6-fast -ListOnly` passed.
- The planned Fast matrix is 12 files x 3 scopes x 3 block sizes, with one
  warmup and three retained repeats: 432 case rows and 864 codec invocations.
- No E6 package or codec process was created; E6 remains gated on E5 completion.

## Runtime progress checkpoint: E5 active (2026-08-29 06:55 +08:00)

- E5 has `290/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` and `auto-k8-b64-r1` are complete. PID 30912 is running
  `auto-b128-r1`; its `dickens` 128 KiB Auto encode is still active and
  responsive.
- `summary.json` is absent; the current matrix must remain intact and resumable.

## Runtime progress checkpoint: E5 active (2026-08-29 06:40 +08:00)

- E5 has `290/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- The final `auto-b128-r1` group is active under PID 30912; `dickens` 64 KiB
  passed and `dickens` 128 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 06:25 +08:00)

- E5 has `289/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k8-b64-r1` is complete. PID 30912 is running the final block-size
  group `auto-b128-r1`; `dickens` 32 KiB passed and `dickens` 64 KiB is the
  active encode.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 06:23 +08:00)

- E5 has `288/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k8-b64-r1` completed all 36 cases. PID 30912 is now running the final
  block-size group `auto-b128-r1`, currently encoding `dickens` 32 KiB.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 06:16 +08:00)

- E5 has `287/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `xml` 64 KiB passed and the final `xml` 128 KiB encode is active.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 06:14 +08:00)

- E5 has `285/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `x-ray` 128 KiB passed and `xml` 32 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 06:07 +08:00)

- E5 has `282/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `webster` 128 KiB passed and `x-ray` 32 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 06:02 +08:00)

- E5 has `279/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `sao` 128 KiB passed and `webster` 32 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 05:43 +08:00)

- E5 has `269/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `osdb` 128 KiB encoding finished and byte-exact decode is active.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 05:39 +08:00)

- E5 has `269/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `osdb` 64 KiB passed and `osdb` 128 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 05:36 +08:00)

- E5 has `267/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `ooffice` 128 KiB passed and `osdb` 32 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment identity and build.

## Runtime progress checkpoint: E5 active (2026-08-29 05:31 +08:00)

- E5 has `265/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `ooffice` 32 KiB passed and its 64 KiB decode is active.
- `summary.json` is absent; preserve the same experiment ID and parameters.

## Runtime progress checkpoint: E5 active (2026-08-29 05:29 +08:00)

- E5 has `264/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `nci` 128 KiB passed and `ooffice` 32 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment ID and parameters.

## Runtime progress checkpoint: E5 active (2026-08-29 05:26 +08:00)

- E5 has `261/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`; `mr`
  128 KiB passed and `nci` 32 KiB is in decode.
- `summary.json` is absent; preserve the same experiment identity and build.

## Runtime progress checkpoint: E5 active (2026-08-29 05:23 +08:00)

- E5 has `260/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete and `auto-k8-b64-r1` is active under PID
  30912; `mr` 64 KiB passed and `mr` 128 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment ID and build.

## Runtime progress checkpoint: E5 active (2026-08-29 05:20 +08:00)

- E5 has `258/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `mozilla` 128 KiB passed and `mr` 32 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment ID and parameters.

## Runtime progress checkpoint: E5 active (2026-08-29 05:15 +08:00)

- E5 has `255/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. The sole runner PID 30912 is in
  `auto-k8-b64-r1`; `dickens` 128 KiB passed and `mozilla` 32 KiB is in decode.
- `summary.json` is absent; preserve the same experiment ID and parameters.

## Runtime progress checkpoint: E5 active (2026-08-29 05:12 +08:00)

- E5 has `254/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` is complete. PID 30912 is running `auto-k8-b64-r1`;
  `dickens` 64 KiB passed and `dickens` 128 KiB is the active encode.
- `summary.json` is absent; preserve the same experiment identity and build.

## Runtime progress checkpoint: E5 active (2026-08-29 05:10 +08:00)

- E5 has `252/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- `auto-k4-b64-r1` completed all 36 cases. PID 30912 remains the sole runner;
  the active child is `auto-k8-b64-r1` processing `dickens` 32 KiB.
- `summary.json` is absent; preserve the same matrix and executable identity.

## Runtime progress checkpoint: E5 active (2026-08-29 05:06 +08:00)

- E5 has `249/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- PID 30912 remains the sole authorized runner. `x-ray` 128 KiB completed;
  `xml` 32 KiB is the next active case under `auto-k4-b64-r1`.
- `summary.json` is absent; preserve the same experiment identity.

## Runtime progress checkpoint: E5 active (2026-08-29 05:02 +08:00)

- E5 has `247/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- PID 30912 remains the sole authorized runner. `x-ray` 64 KiB completed and
  `x-ray` 128 KiB is the active encode under `auto-k4-b64-r1`.
- `summary.json` is absent; preserve the same experiment identity.

## Runtime progress checkpoint: E5 active (2026-08-29 04:59 +08:00)

- E5 has `246/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- PID 30912 remains the sole authorized runner. `webster` 128 KiB completed;
  the active child is encoding `x-ray` 32 KiB under `auto-k4-b64-r1`.
- `summary.json` is absent; preserve the existing experiment ID and parameters.

## Runtime progress checkpoint: E5 active (2026-08-29 04:55 +08:00)

- E5 has `243/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- PID 30912 remains the sole authorized runner. `sao` 128 KiB completed; the
  active child is decoding `webster` 32 KiB under `auto-k4-b64-r1`.
- `summary.json` is absent; preserve the current experiment identity.

## Runtime progress checkpoint: E5 active (2026-08-29 04:50 +08:00)

- E5 has `242/432` completed case rows; every completed row is
  `COMPLETE/PASS`, and failure/error count is zero.
- PID 30912 remains the sole authorized runner. `sao` 64 KiB completed; the
  active child is encoding `sao` 128 KiB under `auto-k4-b64-r1`.
- `summary.json` is not present; keep the current matrix and parameters intact.

## Runtime progress checkpoint: E5 active (2026-08-29 04:46 +08:00)

- E5 has `240/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- PID 30912 is still the sole authorized runner. `samba` 128 KiB completed;
  the active child is encoding `sao` 32 KiB under `auto-k4-b64-r1`.
- `summary.json` is absent; preserve the existing ledger and parameters.

## Runtime progress checkpoint: E5 active (2026-08-29 04:43 +08:00)

- E5 has `239/432` completed case rows; all completed rows are
  `COMPLETE/PASS`, with zero failure/error rows.
- PID 30912 remains the sole authorized runner. `samba` 64 KiB completed and
  `samba` 128 KiB is the active encode under `auto-k4-b64-r1`.
- `summary.json` remains absent; preserve the existing experiment ID.

## Runtime progress checkpoint: E5 active (2026-08-29 04:40 +08:00)

- E5 has `237/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- PID 30912 remains the sole authorized runner. `reymont` 128 KiB completed;
  the active child is encoding `samba` 32 KiB under `auto-k4-b64-r1`.
- `summary.json` is still absent. Preserve the current ledger and do not start
  another matrix.

## Runtime progress checkpoint: E5 active (2026-08-29 04:38 +08:00)

- E5 has `236/432` completed case rows; every recorded row is
  `COMPLETE/PASS`, and the failure count is zero.
- PID 30912 is unchanged as the sole runner; `reymont` 128 KiB is now being
  encoded under `auto-k4-b64-r1`.
- `summary.json` has not appeared. No new experiment or source change is
  permitted before the current matrix completes.

## Runtime progress checkpoint: E5 active (2026-08-29 04:37 +08:00)

- E5 has `235/432` completed case rows; all are `COMPLETE/PASS`, with zero
  failure/error rows.
- PID 30912 remains the single runner. `osdb` 128 KiB completed and the active
  child is encoding `reymont` 64 KiB under `auto-k4-b64-r1`.
- `summary.json` is still absent; keep all post-E5 work gated on completion.

## Runtime progress checkpoint: E5 active (2026-08-29 04:32 +08:00)

- E5 has `233/432` completed case rows; all are `COMPLETE/PASS` and failures
  remain zero.
- PID 30912 is still the sole authorized runner; the active child is encoding
  `osdb` 128 KiB under `auto-k4-b64-r1`.
- `summary.json` is absent. Resume the same experiment ID if interrupted; do
  not start a second matrix.

## Runtime progress checkpoint: E5 active (2026-08-29 04:30 +08:00)

- E5 has `232/432` completed case rows; all rows are `COMPLETE/PASS` and
  failure/error count is zero.
- The single authorized runner (PID 30912) is processing `osdb` at 64 KiB
  under `auto-k4-b64-r1`.
- `summary.json` is not available yet. Preserve the current ledger and resume
  only if the same runner exits unexpectedly.

## Runtime progress checkpoint: E5 active (2026-08-29 04:26 +08:00)

- E5 has `230/432` completed case rows; every recorded row is
  `COMPLETE/PASS`, with zero failure/error rows.
- The single authorized runner (PID 30912) is processing `ooffice` at 128 KiB
  under `auto-k4-b64-r1`; this long Auto case has not yet committed its row.
- `summary.json` remains absent. The matrix, source build, and parameters are
  unchanged; do not launch a duplicate or any post-E5 workload.

## Runtime progress checkpoint: E5 active (2026-08-29 04:18 +08:00)

- E5 has `226/432` completed case rows; all 226 rows are `COMPLETE/PASS` and
  no failure/error row is recorded.
- The single authorized runner (PID 30912) completed `mr` at 128 KiB under
  `auto-k4-b64-r1` and is processing `nci` at 64 KiB.
- `summary.json` is not present yet. Do not launch another runner or begin
  post-E5 documentation, E6, or HZ03 work before the matrix finishes.

## Attachment Experiment Charter (2026-08-28)

The attached R2 decision is now operationalized in
`docs/research/R2_ATTACHMENT_EXPERIMENT_CHARTER_20260828.md`.

- [x] P0: Commit and push the offline K=8 preview evidence as `7c49434`.
- [x] P0: Freeze G1/G2/G4 metrics, pass gates, result locations, and runtime
  costs without changing the codec or launching a corpus experiment. E4
  preflight passed with 44 modes, 43 forced modes, 12 files, 32 KiB internal
  blocks, and 1,056 planned encode/decode invocations.
- [x] P0: Repair the forced Mode-32 PAQ8px RecordModel short-block crash. The
  donor table now has at least two buckets; 1 B, 24 B, 64 B, and 1 KiB
  round-trips are covered by the focused regression path, with old HZ01 and
  Mode-32 archives decoded by the repaired binary.
- [ ] P0: Execute E4 only with the current-build 12-file, 32 KiB, 43-forced
  mode ledger. This is exactly 1,056 encode/decode invocations before retries;
  it is the prerequisite for a measured K=8 claim.
- [ ] P1: Export no-leakage labels, fit an uninstalled candidate ranker, and
  evaluate held-out E5 recall/regret.
- [ ] P1: Run the post-change E6 Fast K=4/executor matrix before making a
  current Fast throughput claim.
- [ ] P2: Run complete Silesia acceptance, then OASum/GPU only through their
  stated owner and evidence gates.

**Current status:** E4 runtime is active under the authorized, resumable ledger
below. Do not launch a second ledger or delete the partial package.

### Resume checkpoint (2026-08-28 13:19 +08:00)

- Ledger ID: `hybridzip-r2-forced-oracle-current-320dd1b`.
- Parent runner: PID 17276, started 12:45; current child: `hybridzip.exe`
  processing the `predictive` `dickens.bin` 32 KiB input.
- Ledger manifest: `results/analysis/r2-complete-ledger/hybridzip-r2-forced-oracle-current-320dd1b/manifest.tsv`.
- Runtime packages: `results/experiments/hybridzip-r2-forced-oracle-current-320dd1b-<mode>/`.
- Current manifest state: Auto and `stored` are `COMPLETE`; `predictive` is
  `TESTING`; the remaining 41 forced packages are `PENDING`; no failure row
  has been recorded. The runner creates each child package atomically and
  skips completed packages on resume.
- Fixed scope: 12 files, 32 KiB input prefix, 32 KiB internal block, 44
  packages, 1,056 planned encode/decode invocations before retries.
- Release executable identity remains the SHA-256 recorded in
  `environment.json`; resume validates ledger ID, manifest dimensions,
  environment fingerprint, codec hash, and existing package rows.

If the process is interrupted, resume from `E:\MIXER\hybridzip` with:

```powershell
.\tools\run_r2_complete_ledger.ps1 `
  -CodecPath .\build\Release\hybridzip.exe `
  -DatasetPath F:\paq8px\silesia `
  -OutputRoot .\results\experiments `
  -LedgerId hybridzip-r2-forced-oracle-current-320dd1b `
  -ScopesKiB 32 -BlockSizeKiB 32 -Resume `
  -AuthorizeRuntimeExperiment
```

The next checkpoint is `predictive COMPLETE/PASS`; after that the runner
advances through the forced modes in manifest order. E5/E6 must remain stopped
until this ledger is complete and derived.

### Runtime progress checkpoint (2026-08-28 13:37 +08:00)

- Manifest counts: `COMPLETE=6`, `TESTING=1`, `PENDING=37`.
- Current package: `donor-match`; the same parent runner (PID 17276) remains
  active and no second codec job was launched.
- Completed package state is retained in each package's `results.csv`; a
  resume invocation will validate and skip those packages before continuing.

### Runtime progress checkpoint (2026-08-28 13:45 +08:00)

- Manifest counts: `COMPLETE=22`, `TESTING=1`, `PENDING=21`.
- Current package: `neural-lstm`; the original parent runner is still active.
- No failures or duplicate ledger were observed; completed package artifacts
  remain the recovery source for `-Resume`.

### Runtime progress checkpoint (2026-08-28 13:49 +08:00)

- Manifest counts: `COMPLETE=23`, `TESTING=1`, `PENDING=20`.
- Current package: `shared-neural-lstm`; the original E4 parent runner remains
  active under the same ledger ID.
- The next post-E4 order is fixed: derive forced-oracle labels, export the
  no-leakage feature set, fit but do not install the candidate ranker, then
  run E5 and the post-change E6 matrix.

### Runtime progress checkpoint (2026-08-28 13:53 +08:00)

- Manifest counts remain `COMPLETE=23`, `TESTING=1`, `PENDING=20`.
- `shared-neural-lstm` has 6 of 12 file rows complete and is still running;
  its package-level `TESTING` state is intentional until all rows pass.
- Partial rows remain on disk for validation or safe replacement by the
  same-package `-Resume` path after interruption.

### Runtime progress checkpoint (2026-08-28 13:56 +08:00)

- `shared-neural-lstm` has advanced to 9 of 12 `COMPLETE/PASS` file rows;
  the codec child remains active.

### Runtime progress checkpoint (2026-08-28 14:01 +08:00)

- Manifest counts: `COMPLETE=24`, `TESTING=1`, `PENDING=19`.
- `shared-neural-lstm` completed; current package is `lstm-compress`.
- No failure row or duplicate ledger was observed.

### Runtime progress checkpoint (2026-08-28 14:06 +08:00)

- Manifest counts: `COMPLETE=27`, `TESTING=1`, `PENDING=16`.
- `lstm-compress`, `delta-of-delta-zstd`, and `bgpt-shared-prior` completed;
  current package is `jax-compress-portable`.
- No failure row or duplicate ledger was observed; E5/E6 remain stopped.

### Runtime progress checkpoint (2026-08-28 14:11 +08:00)

- Top-level status remains `COMPLETE=32`, `TESTING=1`, `PENDING=11`.
- The PAQ-heavy `paq8px-apm` package has 4 of 12 file rows complete and its
  codec child remains CPU-active; no failure or restart was observed.

### Runtime progress checkpoint (2026-08-28 14:14 +08:00)

- `paq8px-apm` has advanced to 9 of 12 completed file rows; its codec child
  remains CPU-active.
- Top-level status remains `COMPLETE=32`, `TESTING=1`, `PENDING=11`; no
  restart or failure was observed.

### Runtime progress checkpoint (2026-08-28 14:16 +08:00)

- Manifest counts: `COMPLETE=35`, `TESTING=1`, `PENDING=8`.
- `paq8px-apm` and `paq8px-linear-prediction` completed; current package is
  `paq8px-similarity`.
- No failure row or duplicate ledger was observed.

### Runtime progress checkpoint (2026-08-28 14:20 +08:00)

- Manifest counts: `COMPLETE=37`, `TESTING=1`, `PENDING=6`.
- `paq8px-similarity-sse` completed; current package is
  `paq8px-generic-sse`.
- No failure row or duplicate ledger was observed.

### Runtime progress checkpoint (2026-08-28 14:23 +08:00)

- Manifest counts: `COMPLETE=38`, `TESTING=1`, `PENDING=5`.
- `paq8px-generic-sse` completed; current package is
  `paq8px-detected-sse`.
- No failure row or duplicate ledger was observed.

### Runtime progress checkpoint (2026-08-28 14:07 +08:00)

- Manifest counts: `COMPLETE=31`, `TESTING=1`, `PENDING=12`.
- `jax-compress-portable`, `ppmd7`, `ppmd8`, and `zpaq` have completed;
  current package is `ctw`.
- No failure row or duplicate ledger was observed; E5/E6 remain stopped.

### Runtime progress checkpoint (2026-08-28 14:09 +08:00)

- Manifest counts: `COMPLETE=32`, `TESTING=1`, `PENDING=11`.
- `ctw` completed; current package is `paq8px-apm`.
- No failure row or duplicate ledger was observed; E5/E6 remain stopped.

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

## 2026-08-28 Candidate Ranker Freeze Interface

- [x] R1: add a deterministic, standard-library offline fitter that accepts
  only the existing file-level no-leakage export and freezes the C++ layout,
  CRC32, and SHA-256 without installing the candidate model.
- [ ] R2: run the fitter on real forced-oracle labels and inspect only the
  held-out split before any C++ model import or E5 promotion claim.

### Status

**R1 in verification.** The fitter intentionally reports validation top-1
tied-winner recall, not K=8 shortlist recall or archive-byte regret. Those
remain E5 acceptance measurements after the forced ledger is complete.

## 2026-08-28 Current-Commit Experiment Start

### Confirmed execution point

- [x] Verify the post-fix commit is published: `main`, `origin/main`, and
  `HEAD` all resolve to `0670cc389f054d3966eb5acfa029729ca72ad6ae`.
- [x] Refresh the attachment-defined E4 preflight against the current Release
  executable `74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F`.
  The 12-file frozen Silesia dataset, 32 KiB scope/internal blocks, Auto, and
  43 forced ratio modes yield 44 child packages and 1,056 planned
  encode/decode invocations. `runtime_started=false`.
- [ ] G1 / E4: Run that exact non-overwriting current-build forced ledger only
  in a dedicated PAQ-heavy window. Acceptance: all 43 forced paths and Auto
  byte-exactly decode and the forced-oracle derivation accepts the ledger.
- [ ] G2/G3 / E5: Export the 28 C++ features with a file-level holdout, fit an
  uninstalled model, then measure K=2/K=4/K=8 against G1. Acceptance for K=8:
  >=99.5% tie-aware recall, >=99.9% byte-weighted recall, <=0.02% aggregate
  regret, and <=16-byte P95 regret.
- [ ] G4 / E6: Re-measure post-change Fast K=4/executor at 32/64/128 KiB;
  retain three post-warmup repeats per cell and require byte-exact decode plus
  >=0.16 MB/s encode/decode. This is a speed result only, not a PAQ-ratio
  result.
- [ ] G5: Run same-input complete-file Silesia ratio acceptance only after
  G1/G2; require HybridZip aggregate complete archive bytes to be strictly
  lower than PAQ8px v216 `-1`.
- [ ] G6/G7: Tencent/OASum and GPU remain deferred. G6 needs owner approval
  for the 1,065,019,104-byte CC-BY-SA-3.0 input; G7 needs a CPU reference and
  a separate >=8 MB/s end-to-end experiment.

### Status

The experiment program is set and E4 has started at its no-codec preflight
checkpoint. No corpus runtime job, PAQ-heavy encode, or throughput measurement
was launched by this checkpoint.

## 2026-08-28 Public Status Boundary Repair

- [x] Correct the public README and product-status hash after the Mode-32
  short-block repair. They now identify `74FF260A...B862D9F` as the current
  Release and label the `CC6DA840...BF191` 528-row ledger as historical.
- [x] State that the router is encoder-only: an HZ02 block records only the
  selected decoder-visible mode and reversible metadata.
- [x] Preserve the important negative claim: E3 PAQ8px input parity is
  complete, while current-build HybridZip ratio, K=8 regret, and post-change
  Fast throughput remain unmeasured until their separate runtime gates.

### Status

The GitHub landing page now matches the attachment evidence boundary. No
archive-format, codec, experiment package, or benchmark result was changed.

## Resume checkpoint: E4 complete / ranker export next (2026-08-28)

- [x] G1 / E4: The current-build forced-oracle ledger is complete. Its
  manifest reports 44/44 `COMPLETE`, covering Auto plus 43 forced ratio modes,
  12 Silesia leading-prefix inputs, 32 KiB input and 32 KiB internal blocks.
  No codec process remains active.
- [x] G1 derivation: Complete archive-byte attribution and tied-winner labels
  were derived under codec SHA-256
  `74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F`.
  The derived package is
  `results/analysis/r2-forced-oracle-derived-320dd1b/`.
- [ ] G2: Export exact C++ `BlockFeaturesV1` values for the 12 oracle rows,
  using an explicit file-level holdout and no codec invocation.
- [ ] G3: Fit a deterministic candidate ranker offline. The model must remain
  `CANDIDATE_FROZEN_NOT_INSTALLED` until E5 passes.
- [ ] G2/G3 checkpoint: After export and fit, record row counts, holdout
  files, model SHA-256, and validation metrics here before any E5 runtime.
- [ ] G4: Run the post-change Fast K=4/executor timing matrix only after the
  ranker step; retain three post-warmup repeats per 32/64/128 KiB cell.
- [ ] G5: Run complete-file same-input Silesia ratio acceptance only after E5.

**Recovery command for the next safe step:**

```powershell
.\tools\export_r2_ranker_training_set.ps1 `
  -ForcedOraclePath .\results\analysis\r2-forced-oracle-derived-320dd1b `
  -DatasetPath F:\paq8px\silesia `
  -FeatureDumpPath .\build\Release\hz_r2_feature_dump.exe `
  -ValidationFiles webster,x-ray,xml `
  -OutputPath .\results\analysis\r2-ranker-training-320dd1b-v1
```

The export is non-overwriting and feature-only. If interrupted, rerun with
the same output path only after inspecting its manifest; do not launch E5.

## Resume checkpoint: ranker export and fit complete (2026-08-28)

- [x] G2: Export completed with 9 training rows and 3 validation rows using
  file-level holdout (`webster`, `x-ray`, `xml`). The exporter invoked
  `hz_r2_feature_dump.exe` 12 times and invoked the codec 0 times.
- [x] G3: Offline deterministic fit completed in
  `results/analysis/r2-ranker-fit-320dd1b-v1/`. The output is explicitly
  candidate-only and is not installed into the production encoder.
- [x] G3 evidence: validation top-1 tied-winner recall is `1.0`; candidate
  model CRC32 is `A0354863`; candidate model SHA-256 is
  `CA1B144EF35E20EC388D739ACE9A1EF92A5E72410D050B5021C3A7F93C62D7B3`.
- [ ] E5 preview: inspect the guarded K=2/K=4/K=8 matrix plan with
  `-ListOnly`; no codec process is allowed during this preview.
- [ ] E5 runtime: only after preview confirmation, run the non-overwriting
  held-out shortlist ledger and compute tie-aware recall, byte-weighted recall,
  aggregate regret, P95 regret, latency, and memory against the complete
  forced-oracle archive bytes.

## Resume checkpoint: E5 runtime authorization corrected (2026-08-28)

- [x] E5 preview: the guarded plan was validated with `runtime_started=false`,
  12 child packages, 36 cases per child, and 864 planned encode/decode
  invocations. The existing complete forced-oracle ledger is linked.
- [ ] E5 runtime: the first launch attempt was rejected before creating a
  package because no explicit file list was supplied. Runtime has not started.
  The corrected launch uses the same stable `ExperimentId` and explicitly
  names all 12 frozen files. It covers Auto, `auto-k2`, `auto-k4`, and
  `auto-k8` at 32/64/128 KiB scopes and block sizes.
- [ ] E5 recovery: if the session or quota interrupts the process, resume with
  the exact command below. The runner validates codec hash, environment
  fingerprint, dimensions, policies, and forced-ledger identity before
  skipping completed child packages.

```powershell
.\tools\run_r2_e5_e6_matrix.ps1 `
  -Stage e5-router `
  -CodecPath .\build\Release\hybridzip.exe `
  -DatasetPath F:\paq8px\silesia `
  -OutputRoot .\results\experiments `
  -ExperimentId hybridzip-r2-e5-router-320dd1b-v1 `
  -SilesiaFiles dickens,mozilla,mr,nci,ooffice,osdb,reymont,samba,sao,webster,x-ray,xml `
  -ScopesKiB 32,64,128 `
  -BlockSizesKiB 32,64,128 `
  -ForcedOracleLedgerPath .\results\analysis\r2-complete-ledger\hybridzip-r2-forced-oracle-current-320dd1b `
  -ListOnly:$false -AuthorizeRuntimeExperiment -Resume
```

The rejected launch is recorded as a pre-runtime validation error; no retry
or duplicate package was produced.

## Runtime progress checkpoint: E5 active (2026-08-28 15:12 +08:00)

- The corrected E5 command created
  `results/experiments/hybridzip-r2-e5-router-320dd1b-v1/` with
  `runtime_started=true`, codec SHA-256
  `74FF260A939B01673667723D8351AAEDB679339610009ECB23C70E373B862D9F`, and
  environment fingerprint
  `6DC773B755B24DBBD0273C4A9E798DF9911284DDE2CFC66E5C7BE4A161D79D5D`.
- The runner has one active codec child and is processing the first child
  package `auto-b32-r1`; the first file `dickens` (32 KiB) reached
  `COMPLETE/PASS`. No duplicate ledger or second codec process exists.
- The process is resumable with the exact stable ExperimentId and the
  explicit 12-file command above. Do not launch E6 until E5 writes a complete
  summary and the forced-oracle matching rows are inspected.

## Runtime progress checkpoint: E5 first child progress (2026-08-28 15:45 +08:00)

- Child `auto-b32-r1` has completed the three `dickens` rows at 32, 64, and
  128 KiB; all three are `COMPLETE/PASS` with byte-exact decoded hashes.
- The same single codec process is advancing to the next file. No retry,
  duplicate ledger, or E6 process was started. Partial rows remain in the
  child `results.csv` and are the recovery source.

## Runtime progress checkpoint: E5 6-row progress (2026-08-28)

- Child `auto-b32-r1` has completed all three scopes for `dickens` and
  `mozilla`, totaling 6/36 rows, each `COMPLETE/PASS` with byte-exact decode.
- The single active codec process has advanced to the next input. The stable
  experiment package remains the only E5 runtime; no duplicate or retry was
  launched.

## Runtime progress checkpoint: E5 12-row progress (2026-08-28)

- Child `auto-b32-r1` has completed all three scopes for `dickens`, `mozilla`,
  `mr`, and `nci`, totaling 12/36 rows; all rows are `COMPLETE/PASS` with
  byte-exact decode.
- The same stable parent session continues with one codec process. No
  replacement package, duplicate ledger, or E6 process was launched.

## Runtime progress checkpoint: E5 14-row progress (2026-08-28)

- Child `auto-b32-r1` now has 14/36 rows complete: all scopes for `dickens`,
  `mozilla`, `mr`, and `nci`, plus `ooffice` at 32/64 KiB. Every completed
  row is `COMPLETE/PASS` with byte-exact reconstruction.
- The parent remains the only E5 runtime and continues with `ooffice` 128 KiB;
  completed CSV rows are durable for `-Resume`.

## Runtime progress checkpoint: E5 24-row progress (2026-08-28)

- Child `auto-b32-r1` has completed all three scopes for eight files through
  `samba`, totaling 24/36 rows. Every completed row is `COMPLETE/PASS` with
  byte-exact reconstruction.
- The same parent session is moving to `sao`; no duplicate process or new
  experiment ID was started.

## Runtime progress checkpoint: E5 27-row progress (2026-08-28)

- Child `auto-b32-r1` has completed all three scopes for nine files through
  `sao`, totaling 27/36 rows; every row is `COMPLETE/PASS` and byte-exact.
- The same child has only `webster`, `x-ray`, and `xml` remaining before the
  runner advances to the next block-size/policy child.

## Runtime progress checkpoint: E5 30-row progress (2026-08-28)

- Child `auto-b32-r1` has completed all scopes for ten files through
  `webster`, totaling 30/36 rows. Every completed row is `COMPLETE/PASS` with
  byte-exact reconstruction.
- Only `x-ray` and `xml` remain in this child; the next child will not start
  until its complete `results.csv` and summary are written.

## Runtime progress checkpoint: E5 34-row progress (2026-08-28)

- `auto-b32-r1` has completed `x-ray` at all three scopes and `xml` at 32 KiB,
  totaling 34/36 rows. All completed rows are `COMPLETE/PASS`.
- Only `xml` 64 and 128 KiB remain in this child. The attachment's proposed
  HZ03 switch is recorded as pending user confirmation; no architecture change
  has been started while E5 continues.

## Runtime progress checkpoint: E5 first child complete (2026-08-28)

- Child `auto-b32-r1` completed all 36/36 cases across the 12 frozen files at
  32/64/128 KiB. Every row is `COMPLETE/PASS` with byte-exact reconstruction.
- The runner wrote the child summary and advanced to child 2/12,
  `auto-k2-b32-r1`. It remains a single stable E5 runtime with no duplicate
  package or process.

## Runtime progress checkpoint: E5 second child 9-row progress (2026-08-28)

- Child `auto-k2-b32-r1` has completed `dickens`, `mozilla`, and `mr` at
  32/64/128 KiB, totaling 9/36 rows, all `COMPLETE/PASS`.
- The parent remains the same single E5 runtime; the completed first child is
  already finalized and will be skipped on any future resume.

## Runtime progress checkpoint: E5 second child complete (2026-08-28)

- Child `auto-k2-b32-r1` completed all 36/36 cases across the 12 frozen files
  at 32/64/128 KiB. Every row is `COMPLETE/PASS` with byte-exact decode.
- The runner finalized child 2/12 and advanced to child 3/12, `auto-k4-b32-r1`.
  The two completed child packages are durable and will be skipped by resume.

## Runtime progress checkpoint: E5 third child 21-row progress (2026-08-28)

- Child `auto-k4-b32-r1` has completed 21/36 rows through `reymont`, covering
  seven files at 32/64/128 KiB. Every completed row is `COMPLETE/PASS`.
- The parent remains a single stable E5 runtime and is processing `samba`;
  completed children 1 and 2 remain finalized recovery artifacts.

## Runtime progress checkpoint: E5 three children complete (2026-08-28)

- Children 1-3 are finalized: `auto-b32-r1`, `auto-k2-b32-r1`, and
  `auto-k4-b32-r1`. Each contains 36/36 `COMPLETE/PASS` rows across the 12
  files and three scopes.
- The runner advanced to child 4/12, `auto-k8-b32-r1`. This is still the same
  stable E5 runtime; no duplicate package or process exists.

## Runtime progress checkpoint: E5 K=8 in progress (2026-08-28 21:37 +08:00)

- Children 1-3 remain finalized at 36/36 `COMPLETE/PASS` each (108 rows).
- Child 4, `auto-k8-b32-r1`, has 29/36 `COMPLETE/PASS` rows; no failure or
  error row is present.
- Exactly one `hybridzip.exe` child is active under the original E5 runner;
  no duplicate ledger, retry package, or E6 process was launched.
- Continue with the same `ExperimentId`; do not start a new matrix or rerun
  completed children. The next checkpoint is child 4 completion or an
  explicit process interruption.

### K=8 row update (2026-08-28 21:39 +08:00)

- `auto-k8-b32-r1` advanced to 31/36 `COMPLETE/PASS` rows; no failure row
  was recorded and the same single codec child remains active.

### K=8 row update (2026-08-28 21:40 +08:00)

- `auto-k8-b32-r1` advanced to 32/36 `COMPLETE/PASS` rows; no failure row
  was recorded and the original single codec child remains active.

### K=8 row update (2026-08-28 21:44 +08:00)

- `auto-k8-b32-r1` advanced to 33/36 `COMPLETE/PASS` rows; the parent E5
  ledger contains 141 passing rows and no failure/error row.

## Runtime progress checkpoint: E5 block-size 64 KiB started (2026-08-28 21:52 +08:00)

- Child 4, `auto-k8-b32-r1`, finalized at 36/36 `COMPLETE/PASS`; E5 now has
  144 passing rows.
- Child 5, `auto-b64-r1`, has its first `dickens` 32 KiB row complete and
  passing (145 total rows); no failure/error row is present.
- The original single E5 runner remains active. Continue with the stable
  `ExperimentId` and do not launch a second matrix.

### 21:56 runtime detail

- `auto-b64-r1` remains at 1/36 `COMPLETE/PASS`; the active child is encoding
  `dickens` at the 64 KiB scope with a temporary archive, CPU-active and no
  failure log. The durable parent total remains 145/432 passing rows.

### 21:59 row update

- `auto-b64-r1` advanced to 2/36 `COMPLETE/PASS`; the E5 parent now has
  146/432 passing rows. No failure/error row or duplicate runner is present.

### 22:00 runtime checkpoint

- E5 durable progress is 146/432 case rows (33.8%), all `COMPLETE/PASS`,
  corresponding to 292/864 encode/decode invocations.
- Four of twelve child packages are finalized; `auto-b64-r1` is at 2/36.
- The single original codec process is CPU-active with no failure/error row.
  Leave it running and resume only with the recorded stable ExperimentId if
  the process is interrupted.

### 22:09 runtime checkpoint

- The E5 parent remains at 146/432 `COMPLETE/PASS` rows; the active child is
  `auto-b64-r1` at 2/36.
- `hybridzip.exe` is still processing the third `dickens` 128 KiB case under
  the original command. No failure row, second ledger, or E6 process exists.
- Per the current user boundary, no Git commit/push or README status rewrite
  is performed until E5 is complete and its summary is available.

### 22:14 runtime checkpoint

- E5 remains at 146/432 `COMPLETE/PASS`; `auto-b64-r1` remains at 2/36.
- Its third case (`dickens`, 128 KiB scope) is still CPU-active in the single
  original codec process (`Responding=True`, no error log or duplicate run).
- Do not terminate or restart the process. README update and GitHub commit/push
  remain gated on the completed E5 summary.

### 22:17 runtime checkpoint

- `auto-b64-r1` advanced to 3/36 `COMPLETE/PASS`; E5 now has 147/432
  passing rows and no failure/error row.
- The original runner remains the only experiment process; the codec child is
  responsive and continues through the fixed case order.

### 22:19 runtime checkpoint

- `auto-b64-r1` is at 3/36 `COMPLETE/PASS`; the E5 parent has 147/432
  passing rows and no failure/error row.
- The codec child remains responsive under the original runner. GitHub commit,
  README rewrite, and any new experiment remain deferred until E5 summary.

### 22:21 runtime checkpoint

- `auto-b64-r1` advanced to 4/36 `COMPLETE/PASS`; E5 total is 148/432
  passing rows with zero failure/error rows.
- The same single codec child remains responsive. No Git operation or new
  experiment is started before the E5 summary exists.

### 22:22 runtime checkpoint

- `auto-b64-r1` advanced to 4/36 `COMPLETE/PASS`; E5 total is 148/432
  passing rows with zero failure/error rows.
- The original single codec process remains responsive and active. E5 summary,
  README update, Git commit, and GitHub push are still pending in that order.

### 22:27 runtime checkpoint

- `auto-b64-r1` advanced to 5/36 `COMPLETE/PASS`; E5 total is 149/432
  passing rows with zero failure/error rows.
- The same single codec child remains active under the original runner. No
  README/Git operation or new experiment is started before E5 summary.

### 22:40 runtime checkpoint

- `auto-b64-r1` advanced to 6/36 `COMPLETE/PASS`; E5 total is 150/432
  passing rows with zero failure/error rows.
- The original runner remains the only active experiment and has advanced past
  `mozilla` 128 KiB. No Git operation or new experiment is started before the
  final E5 summary.

### 22:43 runtime checkpoint

- `auto-b64-r1` completed `mr` 32 KiB and is now at 7/36 rows; E5 total is
  151/432 `COMPLETE/PASS` with no failure/error row.
- The single original codec process continues in fixed order.

### 22:49 runtime checkpoint

- `auto-b64-r1` advanced to 8/36 `COMPLETE/PASS`; E5 total is 152/432
  passing rows with zero failure/error rows.
- The high-cost `mr` 64 KiB case completed (encode 370.660 s, decode
  33.995 s); the original single codec process moved to the next case.

### 23:01 runtime checkpoint

- `auto-b64-r1` completed `mr` 128 KiB and reached 9/36 rows; E5 total is
  153/432 `COMPLETE/PASS`, with zero failure/error rows.
- The same runner advanced to the next fixed case. No Git operation or new
  experiment is started before the full E5 summary.

### 23:05 runtime checkpoint

- E5 advanced to 154/432 `COMPLETE/PASS`; the active `auto-b64-r1` child
  continues in fixed order with zero failure/error rows.
- Only the original runner/codec instance is active. README and GitHub steps
  remain gated on final E5 completion.

### 23:07 runtime checkpoint

- E5 remains at 154/432 `COMPLETE/PASS`, with no failure/error row.
- The active child is still `auto-b64-r1`; the original runner and one codec
  process continue independently after the read-only monitor was stopped.
- No README edit, Git commit/push, E6 run, or HZ03 work was started. These
  remain gated on a complete E5 `summary.json`.

### 23:09 final checkpoint for this turn

- Read-only verification reports E5 at 154/432 `COMPLETE/PASS` rows and zero
  failure/error rows; active child remains `auto-b64-r1`.
- Exactly one actual E5 runner (PID 30912) and one codec child remain active;
  the extra process seen in the raw query was the query shell itself.
- Leave the runner untouched. After `summary.json` appears, perform only the
  requested evidence summary, README update, Git commit/push, and then stop.

### 2026-08-28 denominator correction

- E5 has 432 case rows in total: 12 files x 3 scopes x 3 block sizes x 4
  policies. Each row runs one encode and one decode, giving 864 codec
  invocations.
- An earlier disk check was 155/432 case rows (`COMPLETE/PASS`), equivalent to
  310/864 codec invocations; failure/error rows remained zero.
- A later disk check reached 158/432 case rows (`COMPLETE/PASS`), equivalent to
  316/864 codec invocations; failure/error rows remain zero.
- The latest disk check reached 159/432 case rows (`COMPLETE/PASS`), equivalent
  to 318/864 codec invocations; failure/error rows remain zero.
- Any earlier checkpoint wording that said `X/864 rows` is corrected to
  `X/432 case rows`; `864` is reserved for encode/decode invocations.

### 23:46 runtime checkpoint

- E5 is at 159/432 `COMPLETE/PASS` case rows, equivalent to 318/864 codec
  invocations; failure/error rows remain zero.
- The original E5 runner and one codec child remain active in `auto-b64-r1`.
- `summary.json` is absent. README and GitHub publication remain gated on the
  complete E5 summary; no new experiment has been launched.

### 23:48 runtime checkpoint

- E5 advanced to 160/432 `COMPLETE/PASS` case rows, equivalent to 320/864
  encode/decode invocations; failure/error rows remain zero.
- `auto-b64-r1` remains active with one runner and one codec child. The final
  summary is not present, so README and GitHub actions remain deferred.

### 23:50 runtime checkpoint

- E5 advanced to 160/432 `COMPLETE/PASS` case rows, equivalent to 320/864
  encode/decode invocations; failure/error rows remain zero.
- The original `auto-b64-r1` runner remains active with one codec child and no
  duplicate experiment. `summary.json` is still absent.

### 2026-08-29 00:04 runtime checkpoint

- E5 has 161/432 `COMPLETE/PASS` case rows, equivalent to 322/864
  encode/decode invocations; no failed or incomplete recorded row was found.
- The original runner (PID 30912) remains active with one codec child (PID
  30796) processing `auto-b64-r1` / `osdb` / 128 KiB scope / 64 KiB block.
- `summary.json` is still absent. No duplicate runner, README edit, Git
  operation, E6 timing matrix, or HZ03 work has been started.

### 2026-08-29 00:18 runtime checkpoint

- E5 advanced to 162/432 `COMPLETE/PASS` case rows, equivalent to 324/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- The `auto-b64-r1` `osdb` 128 KiB case completed encode and byte-exact decode;
  the original runner has advanced to `reymont` 32 KiB.
- `summary.json` is still absent, so post-E5 documentation and GitHub actions
  remain gated on completion.

### 2026-08-29 00:26 runtime checkpoint

- E5 advanced to 163/432 `COMPLETE/PASS` case rows, equivalent to 326/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- The same runner is processing `auto-b64-r1` / `reymont` / 64 KiB scope /
  64 KiB block. The child is CPU-active and has not yet written its row.
- `summary.json` remains absent; no new experiment or documentation/publication
  action has been started.

### 2026-08-29 00:31 runtime checkpoint

- E5 advanced to 164/432 `COMPLETE/PASS` case rows, equivalent to 328/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `reymont` at 64 KiB and is now processing its 128 KiB
  case with the same Release executable and environment identity.
- `summary.json` remains absent; README, Git, E6, and HZ03 actions remain
  deferred.

### 2026-08-29 00:46 runtime checkpoint

- E5 advanced to 165/432 `COMPLETE/PASS` case rows, equivalent to 330/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `reymont` at 128 KiB (encode 946.7288133 s,
  decode 33.7774856 s) with byte-exact verification and advanced to `samba`
  32 KiB.
- `summary.json` remains absent; documentation and publication remain gated.

### 2026-08-29 00:51 runtime checkpoint

- After the active child released its CSV lock, E5 was re-read at 166/432
  `COMPLETE/PASS` case rows, equivalent to 332/864 encode/decode invocations;
  failed and incomplete rows remain zero.
- `auto-b64-r1` completed `samba` 32 KiB and advanced to `samba` 64 KiB.
- A transient read-only `Import-Csv` sharing violation occurred while the
  runner appended the previous row; it was not a codec failure and the row
  was confirmed after the file became readable.

### 2026-08-29 01:00 runtime checkpoint

- E5 reached 167/432 `COMPLETE/PASS` case rows, equivalent to 334/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `samba` 64 KiB and started `samba` 128 KiB using the
  same Release executable and environment identity.
- `summary.json` remains absent; no documentation, Git, E6, or HZ03 action has
  started.

### 2026-08-29 01:18 runtime checkpoint

- E5 advanced to 168/432 `COMPLETE/PASS` case rows, equivalent to 336/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `samba` 128 KiB after its long Auto encode and
  byte-exact decode, then started `sao` 32 KiB.
- `summary.json` remains absent and all post-E5 actions remain gated.

### 2026-08-29 01:23 runtime checkpoint

- E5 reached 169/432 `COMPLETE/PASS` case rows, equivalent to 338/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `sao` 32 KiB and advanced to `sao` 64 KiB.
- The stable E5 runner and executable identity are unchanged; no new matrix or
  documentation/publication action has started.

### 2026-08-29 01:32 runtime checkpoint

- E5 reached 170/432 `COMPLETE/PASS` case rows, equivalent to 340/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `sao` at 64 KiB and started its 128 KiB case.
- The three completed `b32` policy packages remain durable; the same runner is
  continuing the remaining Auto cases without a duplicate launch.

### 2026-08-29 01:52 runtime checkpoint

- E5 reached 171/432 `COMPLETE/PASS` case rows, equivalent to 342/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `sao` 128 KiB after the long Auto encode and
  byte-exact decode. The runner is preparing the next fixed case.
- `summary.json` remains absent; no new experiment or post-E5 action started.

### 2026-08-29 01:57 runtime checkpoint

- E5 reached 172/432 `COMPLETE/PASS` case rows, equivalent to 344/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `webster` 32 KiB with byte-exact decode and started
  `webster` 64 KiB.
- `summary.json` remains absent; no new experiment or post-E5 action started.

### 2026-08-29 02:05 runtime checkpoint

- E5 reached 173/432 `COMPLETE/PASS` case rows, equivalent to 346/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `webster` 64 KiB after byte-exact decode and is
  preparing the next fixed case.
- `summary.json` remains absent; no new experiment, code change, or publication
  action has started.

### 2026-08-29 02:06 runtime checkpoint

- E5 remains at 173/432 `COMPLETE/PASS` case rows because the next
  `webster` 128 KiB case is still encoding; no failure row exists.
- `webster` 64 KiB completed with byte-exact decode, and the same runner is
  processing `webster` 128 KiB under `auto-b64-r1`.
- No duplicate runner or parameter change occurred.

### 2026-08-29 03:35 runtime checkpoint

- E5 reached 197/432 `COMPLETE/PASS` case rows, equivalent to 394/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `osdb` 64 KiB and started `osdb` 128 KiB; all
  preceding cases in this child passed byte-exact decode.
- No duplicate runner or experiment identity change occurred.

### 2026-08-29 04:07 runtime checkpoint

- E5 reached 219/432 `COMPLETE/PASS` case rows, equivalent to 438/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k4-b64-r1` completed `dickens` at 32/64/128 KiB and started
  `mozilla` 32 KiB.
- No duplicate runner, code change, or parameter change occurred.

### 2026-08-29 04:10 runtime checkpoint

- E5 reached 221/432 `COMPLETE/PASS` case rows, equivalent to 442/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k4-b64-r1` completed `mozilla` 32 and 64 KiB and is encoding
  `mozilla` 128 KiB.
- No duplicate runner or experiment identity change occurred.

### 2026-08-29 04:01 runtime checkpoint

- E5 reached 215/432 `COMPLETE/PASS` case rows, equivalent to 430/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `xml` 32 and 64 KiB and is processing its final
  `xml` 128 KiB case.
- No duplicate runner, code change, or parameter change occurred.

### 2026-08-29 03:51 runtime checkpoint

- E5 reached 207/432 `COMPLETE/PASS` case rows, equivalent to 414/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `sao` 128 KiB with byte-exact decode and started
  `webster` 32 KiB.
- The original runner and experiment identity remain unchanged.

### 2026-08-29 04:00 runtime checkpoint

- E5 reached 213/432 `COMPLETE/PASS` case rows, equivalent to 426/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `x-ray` 128 KiB with byte-exact decode and
  started the final file's `xml` 32 KiB case.
- No duplicate runner or experiment identity change occurred.

### 2026-08-29 03:46 runtime checkpoint

- E5 reached 205/432 `COMPLETE/PASS` case rows, equivalent to 410/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `sao` 32 KiB with byte-exact decode and started
  `sao` 64 KiB.
- No duplicate runner, code change, or parameter change occurred.

### 2026-08-29 03:53 runtime checkpoint

- E5 reached 209/432 `COMPLETE/PASS` case rows, equivalent to 418/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `webster` 32 and 64 KiB and is encoding
  `webster` 128 KiB.
- No duplicate runner or experiment identity change occurred.

### 2026-08-29 03:47 runtime checkpoint

- E5 reached 206/432 `COMPLETE/PASS` case rows, equivalent to 412/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `sao` 64 KiB with byte-exact decode and started
  `sao` 128 KiB.
- No duplicate runner or experiment identity change occurred.

### 2026-08-29 03:38 runtime checkpoint

- E5 reached 199/432 `COMPLETE/PASS` case rows, equivalent to 398/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `osdb` 128 KiB and `reymont` 32 KiB, and is now
  processing `reymont` 64 KiB.
- The original runner and executable identity remain unchanged.

### 2026-08-29 03:39 runtime checkpoint

- E5 reached 200/432 `COMPLETE/PASS` case rows, equivalent to 400/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `reymont` 64 KiB with byte-exact decode and is
  processing `reymont` 128 KiB.
- No duplicate runner, code change, or parameter change occurred.

### 2026-08-29 03:45 runtime checkpoint

- E5 reached 204/432 `COMPLETE/PASS` case rows, equivalent to 408/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `samba` 128 KiB with byte-exact decode and
  started `sao` 32 KiB.
- No duplicate runner or experiment identity change occurred.

### 2026-08-29 03:41 runtime checkpoint

- E5 reached 201/432 `COMPLETE/PASS` case rows, equivalent to 402/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `reymont` 128 KiB with byte-exact decode and
  started `samba` 32 KiB.
- The original runner and evidence identity remain unchanged.

### 2026-08-29 03:43 runtime checkpoint

- E5 reached 203/432 `COMPLETE/PASS` case rows, equivalent to 406/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `samba` at 32 and 64 KiB and is processing the
  128 KiB case.
- No duplicate runner, code change, or parameter change occurred.

### 2026-08-29 02:21 runtime checkpoint

- E5 reached 174/432 `COMPLETE/PASS` case rows, equivalent to 348/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `webster` 128 KiB after byte-exact decode and
  advanced to `x-ray` 32 KiB.
- `summary.json` remains absent; the same runner continues without a duplicate
  experiment or parameter change.

### 2026-08-29 02:25 runtime checkpoint

- E5 reached 175/432 `COMPLETE/PASS` case rows, equivalent to 350/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `x-ray` 32 KiB with byte-exact decode and is
  preparing the next fixed case.
- `summary.json` remains absent; no new experiment or publication action has
  started.

### 2026-08-29 02:32 runtime checkpoint

- E5 reached 176/432 `COMPLETE/PASS` case rows, equivalent to 352/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `x-ray` 64 KiB with byte-exact decode and started
  `x-ray` 128 KiB.
- The runner continues under the same experiment identity and parameters.

### 2026-08-29 02:45 runtime checkpoint

- E5 reached 177/432 `COMPLETE/PASS` case rows, equivalent to 354/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `x-ray` 128 KiB with byte-exact decode and advanced
  to the final file's `xml` 32 KiB case.
- The same runner, executable, dataset, and matrix dimensions remain active.

### 2026-08-29 02:50 runtime checkpoint

- E5 reached 178/432 `COMPLETE/PASS` case rows, equivalent to 356/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `xml` 32 KiB with byte-exact decode and started
  `xml` 64 KiB.
- The stable runner continues with unchanged experiment identity and
  parameters; `summary.json` remains absent.

### 2026-08-29 02:58 runtime checkpoint

- E5 reached 179/432 `COMPLETE/PASS` case rows, equivalent to 358/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-b64-r1` completed `xml` 64 KiB with byte-exact decode and started
  `xml` 128 KiB.
- This is the final scope in the current Auto block-size child; no new child
  has been launched yet.

### 2026-08-29 03:14 runtime checkpoint

- E5 reached 180/432 `COMPLETE/PASS` case rows, equivalent to 360/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- The `auto-b64-r1` child completed all 36 cases across the 12 Silesia files
  and all three scopes. Its final `xml` 128 KiB row passed byte-exact decode.
- The parent runner is transitioning to the next Auto block-size child; no
  duplicate experiment or parameter change occurred.

### 2026-08-29 03:16 runtime checkpoint

- E5 advanced to 182/432 `COMPLETE/PASS` case rows, equivalent to 364/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- The full `auto-b64-r1` child is complete (36/36). The runner has started
  `auto-k2-b64-r1`, with `dickens` 64 KiB complete and `dickens` 128 KiB
  encoding.
- The same experiment identity and executable hash remain in force.

### 2026-08-29 03:19 runtime checkpoint

- E5 reached 183/432 `COMPLETE/PASS` case rows, equivalent to 366/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `dickens` at 64 and 128 KiB and started
  `mozilla` 32 KiB.
- The original E5 runner remains active with unchanged experiment identity,
  executable, dataset, and matrix dimensions.

### 2026-08-29 03:20 runtime checkpoint

- E5 reached 184/432 `COMPLETE/PASS` case rows, equivalent to 368/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` has advanced through `mozilla` 32 KiB and is decoding its
  64 KiB case after successful encoding.
- No duplicate runner or experiment identity change occurred.

### 2026-08-29 03:22 runtime checkpoint

- E5 reached 187/432 `COMPLETE/PASS` case rows, equivalent to 374/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` has completed through `mozilla` 128 KiB and is processing
  `mr` 64 KiB after the `mr` 32 KiB row passed.
- No duplicate runner, code change, or parameter change occurred.

### 2026-08-29 03:30 runtime checkpoint

- E5 reached 194/432 `COMPLETE/PASS` case rows, equivalent to 388/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed through `ooffice` 64 KiB and is processing
  `ooffice` 128 KiB.
- No duplicate runner or experiment identity change occurred.

### 2026-08-29 03:25 runtime checkpoint

- E5 reached 189/432 `COMPLETE/PASS` case rows, equivalent to 378/864
  encode/decode invocations; failed and incomplete recorded rows remain zero.
- `auto-k2-b64-r1` completed `mr` 128 KiB with byte-exact decode and started
  `nci` 32 KiB.
- The original runner and experiment identity remain active; no duplicate
  matrix was launched.

### 2026-08-29 09:46 runtime checkpoint

- E5 matrix `hybridzip-r2-e5-router-320dd1b-v1` has `312/432`
  `COMPLETE/PASS` case rows, equivalent to `624/864` encode/decode
  invocations; failures remain zero.
- The sole runner is PID `30912`; one codec child is active on
  `sao / 32 KiB / Auto / 128 KiB`.
- `summary.json` is still absent. Preserve the package and do not start E6 or
  a parallel replacement until this matrix reaches completion.

### 2026-08-29 09:53 runtime checkpoint

- E5 reached `314/432` `COMPLETE/PASS` case rows, equivalent to
  `628/864` encode/decode invocations; failures remain zero.
- The sole runner PID `30912` advanced to `sao / 128 KiB / Auto / 128 KiB`.
- The matrix package and executable identity are unchanged; `summary.json` is
  still pending.

### 2026-08-29 10:08 runtime checkpoint

- E5 reached `315/432` `COMPLETE/PASS` case rows, equivalent to
  `630/864` encode/decode invocations; failures remain zero.
- The `sao / 128 KiB / Auto / 128 KiB` case completed byte-exactly and the
  original runner advanced under the same experiment identity.
- `summary.json` is still absent; preserve the active runner and package.

### 2026-08-29 10:10 runtime checkpoint

- E5 reached `316/432` `COMPLETE/PASS` case rows, equivalent to
  `632/864` encode/decode invocations; failures remain zero.
- The original runner advanced to `webster / 64 KiB / Auto / 128 KiB` in the
  same `auto-b128-r1` child package.
- The package remains resumable and `summary.json` is still pending.

### 2026-08-29 10:18 runtime checkpoint

- E5 reached `317/432` `COMPLETE/PASS` case rows, equivalent to
  `634/864` encode/decode invocations; failures remain zero.
- `webster / 64 KiB / Auto / 128 KiB` passed byte-exactly, and the same runner
  advanced to `webster / 128 KiB / Auto / 128 KiB`.
- `summary.json` remains pending until all 12 child packages finish.

### 2026-08-29 10:26 runtime checkpoint

- E5 remains at `317/432` `COMPLETE/PASS` case rows with zero failures.
- The same codec child continues `webster / 128 KiB / Auto / 128 KiB`; CPU
  time is increasing and the process remains responsive.
- No duplicate runner or parameter change occurred; `summary.json` is absent.

### 2026-08-29 10:29 runtime checkpoint

- E5 remains at `317/432` `COMPLETE/PASS` rows with zero failures.
- `webster / 128 KiB / Auto / 128 KiB` is still active; the codec process is
  responsive with increasing CPU time.
- Preserve the single runner and defer final validation until completion.

### 2026-08-29 10:34 runtime checkpoint

- E5 reached `318/432` `COMPLETE/PASS` rows, equivalent to
  `636/864` encode/decode invocations; failures remain zero.
- The `webster` 128 KiB Auto case completed both encode and byte-exact decode.
- The original runner advanced to `x-ray / 32 KiB / Auto / 128 KiB` under the
  unchanged experiment identity.

### 2026-08-29 10:38 runtime checkpoint

- E5 reached `319/432` `COMPLETE/PASS` rows (`638/864` codec invocations),
  with zero failures.
- `x-ray / 32 KiB / Auto / 128 KiB` passed byte-exactly; the runner advanced
  to `x-ray / 64 KiB / Auto / 128 KiB`.

### 2026-08-29 10:42 runtime checkpoint

- E5 reached `320/432` `COMPLETE/PASS` rows (`640/864` codec invocations),
  with zero failures.
- `x-ray / 64 KiB / Auto / 128 KiB` passed byte-exactly; the runner advanced
  to `x-ray / 128 KiB / Auto / 128 KiB`.

### 2026-08-29 10:50 runtime checkpoint

- E5 reached `321/432` `COMPLETE/PASS` rows (`642/864` codec invocations),
  with zero failures.
- `x-ray / 128 KiB / Auto / 128 KiB` completed encode and byte-exact decode.
- The original runner advanced to `xml / 32 KiB / Auto / 128 KiB`, the first
  row of the final Silesia file in `auto-b128-r1`.

### 2026-08-29 10:57 runtime checkpoint

- E5 reached `323/432` `COMPLETE/PASS` rows (`646/864` codec invocations),
  with zero failures.
- `xml / 64 KiB / Auto / 128 KiB` passed byte-exactly; the final
  `auto-b128-r1` row `xml / 128 KiB` is now encoding.

### 2026-08-29 11:07 runtime checkpoint

- E5 reached `324/432` `COMPLETE/PASS` rows (`648/864` codec invocations),
  with zero failures.
- The final `auto-b128-r1` row (`xml / 128 KiB`) passed byte-exactly, so that
  child package is complete.
- The original runner advanced to `auto-k2-b128-r1`, currently decoding
  `dickens / 32 KiB`; no experiment identity or parameter changed.

### 2026-08-29 11:23 runtime checkpoint

- E5 reached `338/432` `COMPLETE/PASS` rows (`676/864` codec invocations),
  with zero failures.
- `auto-k2-b128-r1` completed through `nci / 128 KiB`; the active case is
  `ooffice / 128 KiB / auto-k2 / 128 KiB`.
- The original runner and experiment identities remain unchanged.

### 2026-08-29 11:55 runtime checkpoint

- E5 reached `360/432` `COMPLETE/PASS` rows (`720/864` codec invocations),
  with zero failures.
- `auto-k2-b128-r1` completed all 36 cases byte-exactly.
- The original runner advanced to `auto-k4-b128-r1`, currently encoding
  `dickens / 32 KiB / auto-k4 / 128 KiB`.

### 2026-08-29 12:22 runtime checkpoint

- E5 reached `374/432` `COMPLETE/PASS` rows (`748/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `ooffice / 64 KiB`; the active case is
  `ooffice / 128 KiB / auto-k4 / 128 KiB`.

### 2026-08-29 12:27 runtime checkpoint

- E5 reached `376/432` `COMPLETE/PASS` rows (`752/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `ooffice / 128 KiB` and `osdb / 32 KiB`;
  the active case is `osdb / 64 KiB / auto-k4 / 128 KiB` decode.

### 2026-08-29 12:36 runtime checkpoint

- E5 reached `380/432` `COMPLETE/PASS` rows (`760/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed through `reymont / 64 KiB`; the active case is
  `reymont / 128 KiB / auto-k4 / 128 KiB` decode.

### 2026-08-29 12:41 runtime checkpoint

- E5 reached `383/432` `COMPLETE/PASS` rows (`766/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `reymont / 128 KiB`; the active case is
  `samba / 128 KiB / auto-k4 / 128 KiB`.

### 2026-08-29 12:46 runtime checkpoint

- E5 reached `384/432` `COMPLETE/PASS` rows (`768/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `samba / 128 KiB`; the active case is
  `sao / 32 KiB / auto-k4 / 128 KiB` decode.

### 2026-08-29 12:51 runtime checkpoint

- E5 reached `386/432` `COMPLETE/PASS` rows (`772/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `ooffice / 32,64 KiB`; the active case is
  `sao / 128 KiB / auto-k4 / 128 KiB`.

### 2026-08-29 12:55 runtime checkpoint

- E5 reached `388/432` `COMPLETE/PASS` rows (`776/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `sao / 128 KiB` and `webster / 32 KiB`; the
  active case is `webster / 64 KiB / auto-k4 / 128 KiB` decode.

### 2026-08-29 13:00 runtime checkpoint

- E5 reached `391/432` `COMPLETE/PASS` rows (`782/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `webster / 64,128 KiB` and `x-ray / 32 KiB`;
  the active case is `x-ray / 64 KiB / auto-k4 / 128 KiB`.

### 2026-08-29 13:05 runtime checkpoint

- E5 reached `392/432` `COMPLETE/PASS` rows (`784/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `x-ray / 64 KiB`; the active case is
  `x-ray / 128 KiB / auto-k4 / 128 KiB` decode.

### 2026-08-29 13:09 runtime checkpoint

- E5 reached `395/432` `COMPLETE/PASS` rows (`790/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `x-ray / 128 KiB` and `xml / 32,64 KiB`; the
  active case is its final `xml / 128 KiB` row.

### 2026-08-29 13:15 runtime checkpoint

- E5 reached `398/432` `COMPLETE/PASS` rows (`796/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed all 36 cases byte-exactly.
- The original runner advanced to the final `auto-k8-b128-r1` child,
  currently encoding `dickens / 128 KiB / auto-k8 / 128 KiB`.

### 2026-08-29 13:30 runtime checkpoint

- E5 reached `406/432` `COMPLETE/PASS` rows (`812/864` codec invocations),
  with zero failures.
- `auto-k8-b128-r1` completed `mr / 128 KiB` and `nci / 32 KiB`; the active
  case is `nci / 64 KiB / auto-k8 / 128 KiB` decode.

### 2026-08-29 13:35 runtime checkpoint

- E5 reached `409/432` `COMPLETE/PASS` rows (`818/864` codec invocations),
  with zero failures.
- `auto-k8-b128-r1` completed `nci / 64,128 KiB` and `ooffice / 32 KiB`;
  the active case is `ooffice / 64 KiB / auto-k8 / 128 KiB` decode.

### 2026-08-29 13:52 runtime checkpoint

- E5 reached `417/432` `COMPLETE/PASS` rows (`834/864` codec invocations),
  with zero failures.
- `auto-k8-b128-r1` completed through `reymont / 128 KiB`; the active case is
  `samba / 32 KiB / auto-k8 / 128 KiB`.

### 2026-08-29 14:13 runtime checkpoint

- E5 reached `426/432` `COMPLETE/PASS` rows (`852/864` codec invocations),
  with zero failures.
- `auto-k8-b128-r1` completed `sao / 128 KiB` and all `webster` scopes; the
  active case is `x-ray / 32 KiB / auto-k8 / 128 KiB`.

### 2026-08-29 11:59 runtime checkpoint

- E5 reached `362/432` `COMPLETE/PASS` rows (`724/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `dickens / 32,64 KiB`; the active case is
  `dickens / 128 KiB / auto-k4 / 128 KiB`.

### 2026-08-29 12:04 runtime checkpoint

- E5 reached `365/432` `COMPLETE/PASS` rows (`730/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `dickens / 128 KiB`; the active case is
  `mozilla / 128 KiB / auto-k4 / 128 KiB`.

### 2026-08-29 12:08 runtime checkpoint

- E5 reached `367/432` `COMPLETE/PASS` rows (`734/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` completed `mozilla / 128 KiB` and `mr / 32 KiB`; the
  active case is `mr / 64 KiB / auto-k4 / 128 KiB`.

### 2026-08-29 12:17 runtime checkpoint

- E5 reached `372/432` `COMPLETE/PASS` rows (`744/864` codec invocations),
  with zero failures.
- `auto-k4-b128-r1` has completed through `nci / 128 KiB`; the active case is
  `ooffice / 32 KiB / auto-k4 / 128 KiB` decode.

### 2026-08-29 11:48 runtime checkpoint

- E5 reached `355/432` `COMPLETE/PASS` rows (`710/864` codec invocations),
  with zero failures.
- `auto-k2-b128-r1` completed `webster / 128 KiB` and `x-ray / 32 KiB`; the
  active case is `x-ray / 64 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity remain unchanged.

### 2026-08-29 E5 completion and acceleration checkpoint

- Read-only validation confirms the E5 package
  `results/experiments/hybridzip-r2-e5-router-320dd1b-v1` is complete at
  `432/432` matrix rows and `864/864` codec invocations.
- Every row is `COMPLETE/PASS`; input and decoded SHA-256 values match, and
  `summary.json` is present. No HybridZip/codec process is currently running.
- E5 is closed. The next runtime gate is the post-change E6 Fast K=4 matrix;
  do not rerun E5 or treat historical mode-2 E6 results as current Fast K=4
  evidence.
- Acceleration rule: use `-Resume`, run a small single-cell preflight first,
  then launch the full matrix only once the preflight is byte-exact and its
  executable identity matches the planned package.
- The first E6 preflight exposed a runner-only bug: Fast selected the valid
  `fast-ext` candidate, while the validator incorrectly required `zstd`.
  The validator was narrowed to accept Fast's defined `stored`, `fast-ext`,
  and `lz4` block records; the existing preflight package is retained for
  resume and no codec format change was made.
- The resumed single-cell E6 preflight completed `4/4` timing rows and
  `8/8` codec invocations with byte-exact decode. Retained samples measured
  `0.6025 MB/s` encode and `0.6653 MB/s` decode for this 32 KiB case, above
  the `0.16 MB/s` CPU floor; this is a preflight result, not a corpus claim.
- The formal E6 Fast K=4 single-worker package
  `results/experiments/hybridzip-r2-e6-fast-k4-full-20260829-w1` is complete:
  `432/432` rows (`108` warmup + `324` retained), all byte-exact. The nine
  scope/block cells have minimum encode/decode throughput of `0.5635/0.6112`
  MB/s, above the `0.16 MB/s` floor. Worker count and ranker identity are
  constant at `1` and `00010000|1025B343|4B1AC26C40AD4DA50312FD3B694D7E636FB768C2336FE773BC82D36424C27A4B`.
- The only remaining R2 runtime performance decision is whether a second
  worker-count package is needed for the before/after executor comparison;
  it is not required to rerun the completed E6 single-worker evidence.
- README, `docs/PRODUCT_STATUS.md`, `docs/research/R2_TARGETS_AND_EXECUTION_20260828.md`,
  and `docs/research/R2_IMPLEMENTATION_AUDIT_20260828.md` now point to the
  current E5/E6 packages and distinguish them from historical mode-2 data.
- Verification after edits: PowerShell AST parse passed; CTest passed `4/4`
  (`hz_core_tests`, `hz_pipeline_tests`, `hz_r2_codec_tests`, and
  `hz_structure_routing_tests`).
- F1 artifact checks also pass without codec runtime: `test_fit_r2_fixed_point_ranker.py`,
  `test_r2_ranker_training_set.ps1`, and `test_r2_forced_oracle.ps1` each
  completed successfully. The frozen candidate remains uninstalled because
  shortlist archive-regret evidence for that new model is still absent.
- Full Release CTest completed `18/18` with zero failures in `47.21 s`,
  including HZ01 core/pipeline, all donor backend tests, R2 codec, and
  structure-routing tests.

## 2026-08-29 P0 External-Core Kill Test Pivot

### Goal

Decide, using complete archive bytes and byte-exact reconstruction, whether a
mature external compression core can provide a viable HZ03 foundation before
any further R2 mode, ranker, or GPU work.

### Decisions

- Freeze new R2 encoder modes, current-ranker training/promotion, and GPU
  `LZ_RANS_V1` implementation. Existing HZ01/HZ02 decoders, Fast path,
  ledger tools, and donor ports remain intact.
- Treat Kanzi/libbsc superiority as a hypothesis, not as external evidence.
- Stage the kill test: identical 32/64/128 KiB inputs first; only candidates
  that survive move to representative full-file 1/4/16 MiB superblock runs;
  only the winner then enters the complete-corpus matrix.
- Record all external materials under `E:\MIXER\KU` with upstream URL,
  revision/release, license, download date, and SHA-256. Reuse the existing
  complete Kanzi, PAQ8px, and XZ sources after identity checks; do not
  download duplicates.

### Phases

- [x] P0.0: Commit and push the E5/E6 evidence checkpoint.
  Commit `4d82972b1004509536aafcd588c5dafc529bc14e` is on `origin/main`.
- [x] P0.1: Establish the external-core decision boundary and staged protocol.
- [ ] P0.2: Verify reusable Kanzi/PAQ8px/XZ sources; acquire libbsc into a
  new non-overwriting `E:\MIXER\KU` directory with provenance.
- [ ] P0.3: Build or locate independent CLI executables and run deterministic
  1 KiB compress/decompress smoke tests.
- [ ] P0.4: Run the same-input 32/64/128 KiB external comparison. Do not
  rerun completed E5/E6 packages.
- [ ] P0.5: Run staged representative full-file superblock screening for only
  the candidates justified by P0.4.
- [ ] P0.6: Publish a keep/rebuild/renegotiate decision with all archive-byte,
  timing, memory, and SHA-256 evidence.

### Error Record

- The first curated `git add` command named a non-existent E5-root
  `forced_archive_rows.csv`. The file is in the derived-analysis directory;
  no files were staged by that failed command. The corrected explicit staging
  list passed `git diff --cached --check` before commit.

### Status

**Currently in P0.2** - validating local donor identities and the missing
libbsc acquisition before any external codec runtime.

### 2026-08-29 11:27 runtime checkpoint

- E5 reached `341/432` `COMPLETE/PASS` rows (`682/864` codec invocations),
  with zero failures.
- `auto-k2-b128-r1` completed `ooffice / 128 KiB` and `osdb / 32,64 KiB`;
  the active case is `osdb / 128 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity are unchanged.

### 2026-08-29 11:31 runtime checkpoint

- E5 reached `344/432` `COMPLETE/PASS` rows (`688/864` codec invocations),
  with zero failures.
- `auto-k2-b128-r1` completed `osdb / 128 KiB`; the active case is
  `reymont / 128 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity remain unchanged.

### 2026-08-29 11:44 runtime checkpoint

- E5 reached `352/432` `COMPLETE/PASS` rows (`704/864` codec invocations),
  with zero failures.
- `auto-k2-b128-r1` completed `sao / 128 KiB`; the active case is
  `webster / 64 KiB / auto-k2 / 128 KiB` decode.
- The runner, executable, manifest, and experiment identity remain unchanged.

### 2026-08-29 11:39 runtime checkpoint

- E5 reached `350/432` `COMPLETE/PASS` rows (`700/864` codec invocations),
  with zero failures.
- `auto-k2-b128-r1` completed `samba / 128 KiB`; the active case is
  `sao / 128 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity remain unchanged.

### 2026-08-29 11:34 runtime checkpoint

- E5 reached `347/432` `COMPLETE/PASS` rows (`694/864` codec invocations),
  with zero failures.
- `auto-k2-b128-r1` completed `reymont / 128 KiB`; the active case is
  `samba / 128 KiB / auto-k2 / 128 KiB`.
- The runner, executable, manifest, and experiment identity remain unchanged.

### 2026-08-29 K0 authorization and completion checkpoint

- K0 external-core 1 KiB smoke completed: `11/11` candidates are
  `COMPLETE/PASS`; all encode/decode exit codes are `0` and all decoded
  SHA-256 values match the input.
- The corrected runner is `tools/run_external_core_killtest.ps1`; fixes cover
  Kanzi short-argument parsing, PAQ argument construction, XZ archive copy,
  and stable PowerShell CSV serialization.
- Added the lightweight evidence summary
  `docs/research/HZ03_EXTERNAL_CORE_K0_RESULTS_20260829.md`.
- P0.2 and P0.3 are complete. P0.4 (32/64/128 KiB comparison) remains
  pending explicit continuation; no K1/K2/K3 or source-format change was run.

### Status

**Currently paused at P0.4 authorization boundary** - K0 evidence is committed
and ready for review; the next runtime action is the staged 32/64/128 KiB
comparison only after explicit authorization.
