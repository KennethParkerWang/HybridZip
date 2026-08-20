# HybridZip Product Status

Status date: 2026-08-20

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
| Executable SHA-256 | `2D28B07863E576FE63DF8A5BC5C50C0FDA1DB1A0DD0364BE964981D97E942BD6` |
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
