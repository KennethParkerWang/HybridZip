# HybridZip Product Status

Status date: 2026-08-28

## R2 Current Status

The R2 implementation is active in the same C++17 product and preserves the
HZ01/PROFILE_V1 decoder path. HZ02 currently exposes block modes `0..42` (43
candidate paths), including representation transforms, LZ/coding donors,
specialist PAQ8px graphs, neural profiles, router activation, and multi-coder
selection. The Release executable and the R2 codec test executable compile and
link successfully. The current working-tree Release hash is
`E65526F9DFF3F93844E004D63C7B2A4E4F219B5EAB3F1B3D3ABCF0B301F65003`.
The completed current-hash ledger below remains bound to its earlier
`CC6DA840...` executable and is not silently mixed with this build.

The encoder now also exposes the experimental `--r2-mode=auto-k2`,
`auto-k4`, and `auto-k8` policies. They use deterministic integer byte
features while leaving full `auto`, the archive format, and decoder mode IDs
unchanged. A shared random 1 KiB smoke materialized exactly 2/4/8 candidates,
respectively; every policy selected the 463-byte mode-36
`paq8px-generic-sse` archive and decoded byte-exactly. This is an
implementation gate, not a held-out recall/regret or corpus-level ratio
result; those remain pending E5.

An experimental `--r2-mode=fast` policy is also available for E6 throughput
work. It reuses HZ02 mode 2 with zstd level 3 and is not a ratio-oracle claim.
`tools/run_r2_e5_e6_matrix.ps1` supplies guarded non-overwriting E5/E6 matrix
execution; its list-only plans pass, while full runtime remains unstarted.

On the current working-tree executable, a separate shared random 1 KiB smoke
also preserved both contracts: Fast serialized existing mode 2 (`zstd`) into
662 bytes, and HZ01 produced a 537-byte archive. Both decoded SHA-256 values
equal the 1 KiB input. Evidence is
`results/smoke/r2-e5-e6-compat-1k-20260828-v1/verification-recovery.json`.

The prior evidence Release
`FDE6F9ABC0F831CC9E35BF6B53C24654E06FBB2EE232856924E211A17B04A75B`
has deterministic 1 KiB archive/decode evidence for 42 of the 43 R2 branches.
Mode 8 (`bwt-rlt-zstd`) is the documented exception: random 1 KiB input did
not produce a smaller RLT representation, while suitable 32 KiB evidence
exists. These smokes establish historical branch correctness only; they do not
establish corpus-level ratio, speed, or memory rankings. The active Release has
one new forced-stored random 1 KiB telemetry gate: its 1,084-byte archive
decoded to the exact input SHA-256, and its `selected` and `oracle` CLI values
both equal the complete archive length.

### Current-Hash R2 Ledger

The authorized current-hash ledger is complete at
`results/analysis/r2-complete-ledger/hybridzip-r2-currenthash-cc6d-20260827-r2/`.
It contains Auto plus all 43 forced modes (44 packages), 12 Silesia files at a
32 KiB leading prefix, and 528/528 `COMPLETE/PASS` rows. Every row uses the
active Release hash
`CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191` and
passed complete-archive length, timing, peak-memory, block-attribution, and
byte-exact SHA-256 validation.

The same current Release also passed a separate HZ01 compatibility smoke on a
deterministic 1 KiB input: 537-byte HZ01 archive, 1,024 decoded bytes, and
exact input/decoded SHA-256 equality. Evidence is in
`results/smoke/r2-final-hz01-1k-20260827/verification.json`.

The derived result is 99,720 Auto archive bytes over 393,216 input bytes
(2.028809 bpb). The complete forced-mode oracle is identical: 0 bytes of
aggregate gap and 0 bytes on each of 12 cases. Auto selected
`paq8px-detected-sse` five times and `paq8px-generic-sse` seven times; these
are the only forced modes with an oracle win in this matrix. The strict
analysis bundle and round-review report are in the same ledger directory.

The Silesia experiment runner, family runner, and package validator expose the
complete decoder-visible R2 mode set through mode 42. Their current-hash
execution is covered by the ledger above; the separate segment-oracle runtime
package remains intentionally unrun.

The complete-ledger tooling is now present. `tools/run_r2_complete_ledger.ps1`
creates a non-overwriting manifest for Auto plus all 43 forced R2 paths (44
packages for 43 decoder-visible modes), supports resume by ledger ID, and refuses to
start codec processes unless `-AuthorizeRuntimeExperiment` is supplied.
`tools/derive_r2_complete_ledger.ps1` then validates every declared package,
complete archive length, encode/decode timing and peak memory fields, and input,
archive, and decoded SHA-256 values before writing the Auto/oracle ledger. It
also verifies that each forced package actually emitted its requested HZ02 block
mode for every block, rather than trusting the package name. The tools have
passed PowerShell parse, block-mode negative checks, `-ListOnly`, and the
complete current-hash runtime ledger.

The Silesia runner enforces the same condition immediately after each R2
encode. A successful process that emits an unexpected, malformed, or
wrong-count block record is marked failed before decode, so a long 44-package
run cannot silently attribute a fallback archive to the requested donor.

The intra-file heterogeneity instrumentation is also present at
`tools/run_r2_segment_oracle.ps1`. It partitions one source file by explicit
offset and length, measures all 43 forced modes with complete archive bytes,
time, peak memory, and byte-exact hashes, then writes a per-segment forced-mode
oracle. Auto is optional and its complete-archive gap is recorded separately.
The runner verifies that a forced archive actually records the requested mode
before it can enter the oracle. It is non-overwriting and requires
`-AuthorizeRuntimeExperiment`; its PowerShell parser and no-runtime `-ListOnly`
preflight pass. No segment-oracle runtime package has been produced, and it
does not replace the final R2 ledger.

Post-build branch gates on 2026-08-26 used the prior evidence Release binary
(`FDE6F9ABC0F831CC9E35BF6B53C24654E06FBB2EE232856924E211A17B04A75B`) and one
deterministic random 1 KiB input. Forced mode 41 (`lmic-arithmetic`) produced
a 1768-byte archive and forced mode 42 (`delta-binary-packed-zstd`) produced a
1131-byte archive; both decoded to 1024 bytes with exact input/output SHA-256
equality. Evidence is stored in
`results/smoke/r2-postbuild-1k-20260826/verification.json`. These are branch
correctness gates only and do not establish Auto ranking or corpus performance.

A static coverage audit found all 43 HZ02 `BlockMode` values in the planner,
codec, and archive handling paths. This proves decoder-visible source coverage,
not final runtime or compression-quality completion.

The consolidated metadata-only smoke index at
`results/analysis/r2-smoke-evidence-index-20260826-registry` filters for the
prior evidence-binary hash and finds unique 1 KiB byte-exact evidence for
42/43 HZ02 modes. Its fixed `mode_registry.tsv` includes all 43 names and marks mode 8
(`bwt-rlt-zstd`) as the only missing mode because the random 1 KiB input did
not produce a smaller Kanzi RLT representation; the forced path therefore
emitted no archive. Existing corpus evidence confirms that mode 8 is runnable
for suitable 32 KiB inputs. Historical smoke records use earlier binary hashes
and remain provenance, not current-build evidence.

The remaining prior-evidence-Release gates were executed in three parallel lanes with
unique output directories, separate stdout/stderr logs, current-hash skipping,
and a 60-second process timeout. No Auto, D40, CTest, batch, or larger-block
test was run during this acceleration pass. The detailed mode-8 boundary record
is `results/smoke/r2-postbuild-bwt-rlt-zstd-mode8-1k-20260826-230041-parallel/failure.json`.

The indexer also emits `mode_registry.tsv`, a fixed 43-row registry with the
numeric mode, source-level name, PASS/MISSING status, and accepted evidence
path. It is metadata-only and does not run the codec.

The latest forced mode 29 (`zpaq`) gate produced a 1365-byte archive
(10.664063 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in `results/smoke/r2-postbuild-zpaq-1k-20260826/verification.json`.

The latest forced mode 25 (`bgpt-shared-prior`) gate produced a 1759-byte
archive (13.742188 bpb), decoded to 1024 bytes, and matched the input SHA-256
exactly. Evidence is stored in
`results/smoke/r2-postbuild-bgpt-shared-prior-1k-20260826/verification.json`.

The latest forced mode 24 (`delta-of-delta-zstd`) gate produced a 1099-byte
archive (8.585938 bpb), decoded to 1024 bytes, and matched the input SHA-256
exactly. Evidence is stored in
`results/smoke/r2-postbuild-delta-of-delta-zstd-1k-20260826/verification.json`.

The latest forced mode 23 (`lstm-compress`) gate produced a 1098-byte archive
(8.578125 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in
`results/smoke/r2-postbuild-lstm-compress-1k-20260826/verification.json`.

The latest forced mode 22 (`shared-neural-lstm`) gate produced a 1099-byte
archive (8.585938 bpb), decoded to 1024 bytes, and matched the input SHA-256
exactly. Evidence is stored in
`results/smoke/r2-postbuild-shared-neural-lstm-1k-20260826/verification.json`.

The latest forced mode 21 (`neural-lstm`) gate produced a 1094-byte archive
(8.546875 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in
`results/smoke/r2-postbuild-neural-lstm-1k-20260826/verification.json`.

The latest forced mode 20 (`cmix-word-zstd`) gate produced a 1403-byte
archive (10.960938 bpb), decoded to 1024 bytes, and matched the input SHA-256
exactly. Evidence is stored in
`results/smoke/r2-postbuild-cmix-word-zstd-1k-20260826/verification.json`.

The latest forced mode 19 (`brotli-text`) gate produced a 1088-byte archive
(8.5 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in
`results/smoke/r2-postbuild-brotli-text-1k-20260826/verification.json`.

The latest forced mode 18 (`flac-residual`) gate produced a 1182-byte archive
(9.234375 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in
`results/smoke/r2-postbuild-flac-residual-1k-20260826/verification.json`.

The latest forced mode 17 (`jpegls`) gate produced a 1333-byte archive
(10.414063 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in
`results/smoke/r2-postbuild-jpegls-1k-20260826/verification.json`.

The latest forced mode 16 (`record-transpose-zstd`) gate produced a 1099-byte
archive (8.585938 bpb), decoded to 1024 bytes, and matched the input SHA-256
exactly. Evidence is stored in
`results/smoke/r2-postbuild-record-transpose-zstd-1k-20260826/verification.json`.

The latest forced mode 15 (`bcj2-zstd`) gate produced a 1119-byte archive
(8.742188 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in
`results/smoke/r2-postbuild-bcj2-zstd-1k-20260826/verification.json`.

The latest forced mode 14 (`rans`) gate produced a 1828-byte archive
(14.28125 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in
`results/smoke/r2-postbuild-rans-1k-20260826/verification.json`.

The latest forced mode 13 (`fastpfor`) gate produced a 1106-byte archive
(8.640625 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Because the conventional output directory was already occupied, evidence is
stored in `results/smoke/r2-postbuild-fastpfor-mode13-1k-20260826/verification.json`.

The latest forced mode 28 (`ppmd8`) gate produced a 1194-byte archive
(9.328125 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in `results/smoke/r2-postbuild-ppmd8-1k-20260826/verification.json`.

The latest forced mode 27 (`ppmd7`) gate produced a 1193-byte archive
(9.320313 bpb), decoded to 1024 bytes, and matched the input SHA-256 exactly.
Evidence is stored in `results/smoke/r2-postbuild-ppmd7-1k-20260826/verification.json`.

The latest forced mode 26 (`jax-compress-portable`) gate produced a 1141-byte
archive (8.914063 bpb), decoded to 1024 bytes, and matched the input SHA-256
exactly. Evidence is stored in
`results/smoke/r2-postbuild-jax-compress-portable-1k-20260826/verification.json`.

The current continuation record is [task_plan.md](../task_plan.md)
and the donor audit notes are [notes.md](../notes.md).

Donor warehouse validation passed on 2026-08-26 with 2506 checks: 21 donor
manifests, 18 port evidence records, 17 Git revisions/origins, one release
snapshot, three source archives, and 21 license evidence hashes. The audit did
not identify another complete model-free C++17 decoder closure for immediate
integration.

HybridZip's first-generation C++17 product path is implemented and verified on
the current Windows toolchain. The Release executable, four active experts,
streaming encoder/decoder, fixed HZ01 contract, five repository tests, and the
nine-input product/baseline matrix, and formal 36-case Silesia Experiment
Ledger package are present and independently validated.

## Build And Executable

| Item | Verified state |
| --- | --- |
| Project version | CMake project `1.0.0`; archive profile `PROFILE_V1` |
| Build status | Release build present; configuration and build completed successfully |
| Build command | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` |
| Compile command | `cmake --build build --config Release` |
| Test command | `ctest --test-dir build -C Release --output-on-failure` |
| Environment | Windows 10 Pro 10.0.19045 x64 |
| Toolchain | CMake 4.3.2, Ninja 1.13.2, GCC 16.1.0 |
| Executable | `E:\MIXER\hybridzip\build\Release\hybridzip.exe` |
| Executable size | 3,220,993 bytes |
| HZ01 baseline executable SHA-256 | `2D28B07863E576FE63DF8A5BC5C50C0FDA1DB1A0DD0364BE964981D97E942BD6` |
| CLI | `hybridzip c <input> <archive>` and `hybridzip d <archive> <output>` |

## Active Product Contract

| Stage | Active implementation |
| --- | --- |
| History | `ByteHistory`, 8 MiB capacity |
| Expert 1 | NGram orders 0-4, online sparse continuation counts |
| Expert 2 | adapted cmix PPMD, order 12, 64 MiB model memory |
| Expert 3 | byte-native Match, 8-byte context, `2^20` slots, 8 MiB window |
| Expert 4 | adapted cmix Online LSTM, 200 cells, 2 layers, horizon 100 |
| Mixer | `AdaptiveLinearMixer`, four initial weights of 0.25, `eta = 0.5` |
| Probability | normalized 256-way byte distribution |
| CDF | deterministic 24-bit CDF, total `2^24`, every symbol positive |
| Arithmetic coder | Project Nayuki reference coder, 32-bit state |
| Archive | HZ01 version 1, 40-byte little-endian header, `profile_id = 1` |
| Encoder/decoder | streaming single-file paths with identical model lifecycle |

The normative contracts are [FORMAT.md](FORMAT.md) and
[PROFILE_V1.md](PROFILE_V1.md).

## Test Status

Fresh verification on 2026-08-20: **5/5 CTest tests passed** in 0.40 seconds.

| Test | Covered behavior | Status |
| --- | --- | ---: |
| `hz_core_tests` | probability/CDF, ByteHistory, HZ01 header, mixer, arithmetic round-trip | PASS |
| `hz_lstm_predictor_tests` | seeded Online LSTM determinism and lifecycle | PASS |
| `hz_predictor_tests` | NGram capacity/update and Match learning/reset | PASS |
| `hz_ppmd_predictor_tests` | adapted full-byte PPMD prediction/update | PASS |
| `hz_pipeline_tests` | four-expert PROFILE_V1 lifecycle and identical CDF evolution | PASS |

Additional byte-exact checks:

| Input | Archive result | Encode s | Decode s | Verification |
| --- | ---: | ---: | ---: | --- |
| Empty file, 0 bytes | 41-byte HZ01 | not retained | not retained | decoded size 0 |
| 221-byte text smoke | 183 bytes | 0.239568 | 0.266007 | matching SHA-256 |
| Dickens 128 KiB prefix | 37,483 bytes, ratio 0.285973, 2.287781 bpb | 77.747313 | 77.867495 | matching SHA-256 `20E33F...D59872` |

The ad hoc 128 KiB memory probe was read after process exit and returned zero;
that invalid value is intentionally excluded. The product runner below samples
`PeakWorkingSet64` while each process is active.

## Product And Baseline Results

Result artifacts:

| Artifact | Contents | Status |
| --- | --- | ---: |
| [PRODUCT_CORPUS.md](PRODUCT_CORPUS.md) | nine input classes and prefix selections | complete |
| [product_manifest.tsv](../results/product_manifest.tsv) | source paths, selected byte counts, input SHA-256 | 9 rows |
| [product_test.tsv](../results/product_test.tsv) | HybridZip size, time, throughput, and peak memory | 9 rows |
| [baseline_tools.tsv](../results/baseline_tools.tsv) | executable versions, parameters, paths, and SHA-256 | 7 rows including HybridZip |
| [baseline_test.tsv](../results/baseline_test.tsv) | six baseline codecs on the same inputs | 54 rows |

HybridZip aggregate over 251,589 input bytes:

| Metric | Measured value |
| --- | ---: |
| HZ01 bytes | 80,675 |
| Weighted ratio | 0.320662 |
| Weighted bits per byte | 2.565295 |
| Sum of encode wall times | 170.030261 s |
| Sum of decode wall times | 163.049962 s |
| Maximum sampled peak working set | 129.617 MiB |
| SHA-256 round-trips | 9/9 PASS |

Same-input aggregate baseline sizes:

| Codec | Archive bytes | Weighted ratio | Weighted bpb | SHA-256 round-trips |
| --- | ---: | ---: | ---: | ---: |
| gzip 1.14 `-9` | 92,083 | 0.366006 | 2.928045 | 9/9 PASS |
| zstd 1.5.7 `-19 -T1` | 84,817 | 0.337125 | 2.697002 | 9/9 PASS |
| brotli 1.2.0 `-q 11` | 77,553 | 0.308253 | 2.466022 | 9/9 PASS |
| xz 5.6.4 `-9e -T1` | 79,580 | 0.316310 | 2.530476 | 9/9 PASS |
| 7-Zip 26.00 `-t7z -mx=9 -mmt=1` | 78,499 | 0.312013 | 2.496103 | 9/9 PASS |
| PAQ8PX v216 `-1` | 62,137 | 0.246978 | 1.975826 | 9/9 PASS |

An independent result audit checked all 63 table rows against 63 archives and
63 decoded files. Row counts, archive sizes, decoded SHA-256 values, and every
ratio/bpb formula passed with zero errors.

### Silesia Experiment Ledger

| Item | Verified result |
| --- | --- |
| Experiment ID | `hybridzip-profile-v1-silesia-prefix-20260820-031351-2d28b078` |
| Canonical dataset path | `F:\paq8px\silesia` |
| Coverage | 12 files x 32/64/128 KiB = 36 rows |
| Status | 36/36 `COMPLETE/PASS` |
| Input/archive bytes | 2,752,512 / 810,958 |
| Weighted ratio / bpb | 0.294625 / 2.356998 |
| Encode/decode wall time | 1,598.059150 / 1,603.865058 s |
| Maximum sampled peak working set | 129.617 MiB |
| Independent validation | row/schema/coverage, 108 artifact lengths and SHA-256, exit codes, codec hash: PASS |
| Package | `results/experiments/hybridzip-profile-v1-silesia-prefix-20260820-031351-2d28b078` |
| `experiment.json` SHA-256 | `146216DCB3EB9213A4DD308B479A2F2F164E34DCACCC7D3E99EB77512CE12F1C` |
| `results.csv` SHA-256 | `8FDED00651A5BBC68A46A8B3CC3637008CF5C737A97B00199C268EE583675551` |

The directory is a validated local evidence package, not a standalone
SchemaVersion 2 workspace. See [DATASET_PROVENANCE.md](DATASET_PROVENANCE.md)
before distributing corpus-derived artifacts or merging results into the
existing Experiment Ledger workspace.

## Known Engineering Limits

- The CLI accepts one regular input file and creates one output file. Existing
  output and `<output>.tmp` paths are rejected.
- HZ01 does not store a checksum, CRC, corruption-recovery data, authentication,
  file metadata, directory structure, or random-access index. Payload damage
  or truncation is not guaranteed to be detected.
- PROFILE_V1 is fixed and single-threaded. It has no preprocessing, file-type
  router, GPU path, or checkpoint support.
- The nine-case product corpus uses exact prefixes for large source files and
  one run per codec/case. Its measurements are engineering observations, not
  statistical or paper-grade comparisons.
- The Online LSTM uses floating-point cmix computations. The current binary and
  tested toolchain are deterministic; cross-compiler and cross-ISA bitstream
  identity has not been established.
- MinGW embeds the link time in the PE header. A relink can be behaviorally
  identical while producing a different executable SHA-256. The formal Ledger
  identifies the preserved Release binary, not a byte-reproducible fresh build.
- The inherited cmix PPMD arena-exhaustion path remains an unvalidated risk for
  long random streams. Current verified inputs do not establish unlimited-size
  reliability.

## Deferred Strict Validation

The source engineering report explicitly defers these items from the
first-generation completion boundary:

- large-scale byte-exact regression
- fuzz testing
- cross-platform determinism certification
- archive-corruption test suite
- formal correctness proof
- paper-grade experimental protocol and statistical analysis

Source provenance and license details are in [SOURCES.md](SOURCES.md),
[NOTICE.md](../NOTICE.md), and [LICENSE](../LICENSE).
