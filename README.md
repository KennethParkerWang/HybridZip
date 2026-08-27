# HybridZip

HybridZip is a C++17 research compressor for one regular file at a time. It
keeps the legacy HZ01 format and PROFILE_V1 decoder path for regression
compatibility, and adds the block-oriented HZ02 R2 portfolio described below.
The HZ01 path uses four online, byte-native probability experts, an adaptive
mixer, a deterministic 24-bit CDF, and a 32-bit arithmetic coder. The decoder
reconstructs the same model state from the archive profile, seed, and decoded
byte prefix; no model file is stored in the archive.

The current R2 build adds the block-oriented HZ02 portfolio: existing
decoder-visible candidate paths `0..42` plus the append-only experimental
`MODE_FAST_EXT_V1` path at ID `43`. The portfolio spans representation
transforms, LZ/entropy donors, specialist PAQ8px branches, neural profiles,
and router-controlled Auto selection. The encoder also has experimental
`auto-k8` and `fast` policies; Fast is now a K=4 policy over stored, Mode-43
zstd extensions (raw or reversible transform), and LZ4. See
[docs/PRODUCT_STATUS.md](docs/PRODUCT_STATUS.md) for the
current evidence boundary and known limitations.

R2 follows a donor-first integration rule. Donor revisions, accepted code
subsets, provenance, and license boundaries are recorded in
[docs/DONORS.md](docs/DONORS.md) and
[docs/LICENSE_MATRIX.md](docs/LICENSE_MATRIX.md).

The HZ01 baseline Release executable, five HZ01 baseline tests (within the 18
CTest targets registered by the current build), nine-input product matrix, and
36-case Silesia ledger are complete. The active R2 Release has a complete
current-hash ledger for Auto plus all 43 forced modes: 44 packages, 528 rows,
and 528/528 byte-exact round trips. On the declared 12-file, 32 KiB Silesia
prefix matrix, Auto totals 99,720 archive bytes (2.028809 bpb) and matches the
complete forced-mode oracle with a zero-byte gap in every case. Auto selected
`paq8px-detected-sse` five times and `paq8px-generic-sse` seven times. This is
an engineering result for one prefix size; it is not a global ranking or a
reason to delete the remaining donor paths.

The R2-A through R2-D continuation plan and the ledger decision are tracked in
[task_plan.md](task_plan.md). The exact ledger, strict analysis bundle, and
round-review report are under
`results/analysis/r2-complete-ledger/hybridzip-r2-currenthash-cc6d-20260827-r2/`.
The phase labels describe execution order; they do not reduce the final
portfolio scope.

```text
HZ01 compatibility path
ByteHistory
  -> NGram + PPMD + Match + Online LSTM
  -> AdaptiveLinearMixer
  -> normalized 256-way probability
  -> deterministic CDF24
  -> Project Nayuki ArithmeticCoder (32-bit state)
  -> HZ01

HZ02 R2 path
input -> structure/representation candidates
      -> LZ, specialist, neural, and multi-coder candidates
      -> decoder-visible Auto router or forced mode
      -> checksummed HZ02 blocks
```

## Build

Requirements:

- CMake 3.20 or newer
- A C++17 compiler
- CTest when building the default test targets

From the repository root:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The executable is written to:

```text
build\Release\hybridzip.exe
```

The current Windows Release was built with GCC 16.1.0 and Ninja 1.13.2. The
same source is standard C++17, but cross-compiler bitstream identity has not
been certified.

## Compress

```powershell
.\build\Release\hybridzip.exe c .\input.bin .\output.hz
```

## Decompress

```powershell
.\build\Release\hybridzip.exe d .\output.hz .\restored.bin
```

## R2 HZ02 Path

R2 compression is selected explicitly with `--profile=r2`. Auto evaluates the
enabled candidate portfolio for each block; a decoder-visible path can be
forced for branch experiments. HZ02 archives use the same decompression
command because the archive header selects the decoder.

```powershell
.\build\Release\hybridzip.exe c --profile=r2 --r2-mode=auto .\input.bin .\output.hz
.\build\Release\hybridzip.exe c --profile=r2 --r2-mode=zstd .\input.bin .\output-zstd.hz
.\build\Release\hybridzip.exe d .\output.hz .\restored.bin
```

Run `hybridzip --help` for the complete decoder-visible mode list and optional
block/LZ parameters.

Input and output must be different paths. HybridZip refuses to overwrite an
existing output or an existing `<output>.tmp`; successful writes are renamed
from the temporary path.

## Archive And Profile

[HZ01](docs/FORMAT.md) is a single-file archive with a 40-byte, little-endian
header followed by an arithmetic-coded payload. The header fixes version 1,
`profile_id = 1`, the original size, model seed, CDF width, and coder state
width.

[PROFILE_V1](docs/PROFILE_V1.md) is part of the archive contract and has no
command-line tuning options:

| Component | Fixed configuration |
| --- | --- |
| NGram | orders 0-4, sparse online counts, about 64 MiB target |
| PPMD | order 12, 64 MiB model memory |
| Match | 8-byte context, `2^20` slots, 8 MiB history, minimum match 3 |
| Online LSTM | 200 cells/layer, 2 layers, horizon 100, learning rate 0.03 |
| Mixer | four equal initial weights, adaptive log update, `eta = 0.5` |
| CDF | 256 symbols, total `2^24`, deterministic largest remainder |
| Arithmetic coder | Project Nayuki reference coder, 32-bit state |
| Archive | HZ01 version 1, `profile_id = 1` |

## Sources And License

| Source | Revision | Role | License |
| --- | --- | --- | --- |
| cmix | `1d95fe95381a01442fceab585375cdec7c06922f` | adapted PPMD and Online LSTM | GPLv3 |
| Project Nayuki Reference Arithmetic Coding | `ab6ee50afec04d235a4b82d17f407f0fd2b42e9a` | arithmetic coder, bit streams, frequency interface | MIT |
| Nacrith-GPU | `ff29c42e5cfa77d7c00641880e99713644adc923` | algorithmic reference for independently written NGram and mixer | Apache-2.0 |

HybridZip is distributed under [GNU GPL version 3](LICENSE) because it contains
modified cmix source. Exact upstream locations and adaptations are recorded in
[SOURCES.md](docs/SOURCES.md); required attributions are in
[NOTICE.md](NOTICE.md).

## HZ01 Baseline Measured Results

The measurements in this section are the completed HZ01/PROFILE_V1 baseline.
The separate R2 Auto/oracle archive-byte ledger is recorded in
[docs/PRODUCT_STATUS.md](docs/PRODUCT_STATUS.md) and the current-hash ledger
directory described above.

The product corpus contains nine data classes and 251,589 input bytes. Large
sources are exact leading-byte prefixes, not full-corpus runs; selections and
SHA-256 identities are in [PRODUCT_CORPUS.md](docs/PRODUCT_CORPUS.md) and
[product_manifest.tsv](results/product_manifest.tsv).

`ratio` means `archive bytes / original bytes`, and `bpb` is `8 * ratio`.
Times are one encode and one decode process per case. Peak memory is the larger
sampled `PeakWorkingSet64` from those two processes. All nine HybridZip
round-trips passed SHA-256 equality.

| Input | Original bytes | HZ01 bytes | Ratio | bpb | Encode s | Decode s | Peak MiB |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `plain-text.bin` | 16,384 | 6,019 | 0.367371 | 2.938965 | 14.095725 | 14.936327 | 129.617 |
| `source-code.cpp` | 5,829 | 1,199 | 0.205696 | 1.645565 | 5.636642 | 5.382560 | 129.371 |
| `json-xml.xml` | 16,384 | 1,533 | 0.093567 | 0.748535 | 14.602075 | 11.774899 | 129.363 |
| `binary-executable.bin` | 16,384 | 7,582 | 0.462769 | 3.702148 | 11.036124 | 10.714312 | 129.613 |
| `database-records.bin` | 16,384 | 8,469 | 0.516907 | 4.135254 | 10.377916 | 9.888044 | 129.602 |
| `image-xray.bin` | 16,384 | 9,464 | 0.577637 | 4.621094 | 9.637803 | 9.970146 | 129.613 |
| `audio-media.wav` | 16,384 | 11,982 | 0.731323 | 5.850586 | 9.647838 | 11.038627 | 129.613 |
| `compressed-high-entropy.hz` | 16,384 | 16,424 | 1.002441 | 8.019531 | 11.533955 | 11.001227 | 129.598 |
| `large-mixed.bin` | 131,072 | 18,003 | 0.137352 | 1.098816 | 83.462184 | 78.343821 | 129.613 |
| **Total / weighted** | **251,589** | **80,675** | **0.320662** | **2.565295** | **170.030261** | **163.049962** | **129.617 max** |

The same nine inputs were also run once through six available baselines. The
table reports weighted size metrics and sums of per-case wall times. All 54
baseline round-trips passed SHA-256 equality.

| Codec | Version and parameters | Archive bytes | Ratio | bpb | Encode s | Decode s | Peak MiB |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| HybridZip | `1.0.0-profile-v1` | 80,675 | 0.320662 | 2.565295 | 170.030261 | 163.049962 | 129.617 |
| gzip | `1.14 -9` | 92,083 | 0.366006 | 2.928045 | 0.522088 | 0.524277 | 6.340 |
| zstd | `1.5.7 -19 -T1` | 84,817 | 0.337125 | 2.697002 | 0.920949 | 0.779676 | 6.406 |
| brotli | `1.2.0 -q 11` | 77,553 | 0.308253 | 2.466022 | 0.839269 | 0.541620 | 12.543 |
| xz | `5.6.4 -9e -T1` | 79,580 | 0.316310 | 2.530476 | 0.536516 | 0.454160 | 33.746 |
| 7-Zip | `26.00 -t7z -mx=9 -mmt=1` | 78,499 | 0.312013 | 2.496103 | 0.523565 | 0.515041 | 7.980 |
| PAQ8PX | `v216 -1` | 62,137 | 0.246978 | 1.975826 | 40.168707 | 37.674753 | 1,147.449 |

These are descriptive engineering measurements on small prefixes, not a
statistical or paper-grade benchmark. Exact rows are in
[product_test.tsv](results/product_test.tsv),
[baseline_test.tsv](results/baseline_test.tsv), and
[baseline_tools.tsv](results/baseline_tools.tsv).

### HZ01 Silesia Baseline Ledger

The formal import package covers all 12 Silesia members at 32, 64, and 128 KiB
with one fixed PROFILE_V1 variant and one repeat. All 36 rows are
`COMPLETE/PASS`; an independent validator re-read every artifact length and
SHA-256 after the run.

| Metric | Result |
| --- | ---: |
| Input bytes | 2,752,512 |
| HZ01 bytes | 810,958 |
| Weighted ratio | 0.294625 |
| Weighted bpb | 2.356998 |
| Sum of encode wall times | 1,598.059150 s |
| Sum of decode wall times | 1,603.865058 s |
| Maximum sampled peak working set | 129.617 MiB |
| Byte-exact round-trips | 36/36 PASS |

Package:
[`results/experiments/hybridzip-profile-v1-silesia-prefix-20260820-031351-2d28b078`](results/experiments/hybridzip-profile-v1-silesia-prefix-20260820-031351-2d28b078)

The package uses canonical dataset path `F:\paq8px\silesia`, matching the
existing Experiment Ledger DatasetIdentity. Its final `experiment.json` and
`results.csv` SHA-256 values are `146216DCB3EB9213A4DD308B479A2F2F164E34DCACCC7D3E99EB77512CE12F1C`
and `8FDED00651A5BBC68A46A8B3CC3637008CF5C737A97B00199C268EE583675551`.
It is a local evidence package, not a standalone GUI workspace import. Merge
its validated rows into the existing SchemaVersion 2 workspace before UI
import, and follow the corpus distribution boundary in
[DATASET_PROVENANCE.md](docs/DATASET_PROVENANCE.md). Corpus-derived evidence
directories are excluded from normal Git staging but remain present locally.

## Known Limitations

- HZ01 stores one file and does not preserve names, timestamps, permissions, or
  directory structure.
- HZ01 has no checksum, CRC, corruption recovery, authentication, random
  access, or metadata channel. A truncated or modified payload is not
  guaranteed to be detected.
- HZ01/PROFILE_V1 is fixed; that compatibility path has no model selection,
  preprocessing, file-type routing, multithreading, GPU path, or checkpointing.
  These constraints do not describe the HZ02 R2 portfolio.
- PROFILE_V1 is CPU-intensive in the current implementation. The nine-case run
  above summed to 170.030261 s encode and 163.049962 s decode.
- The floating-point Online LSTM is deterministic in the released binary and
  tested toolchain. Cross-compiler and cross-ISA bitstream identity is not yet
  validated.
- The MinGW linker embeds the link time in the PE header. Relinking unchanged
  sources can preserve behavior while changing the executable SHA-256. The
  formal Ledger is bound to the preserved Release binary above; a fresh binary
  requires a new result package if its hash differs.
- The inherited cmix PPMD arena-exhaustion path remains an unvalidated risk for
  long random streams; the verified product cases do not exercise unlimited
  input lengths.
- Large-scale byte-exact regression, fuzzing, archive-corruption testing,
  formal correctness work, and paper-grade evaluation are deferred. See
  [PRODUCT_STATUS.md](docs/PRODUCT_STATUS.md) for the exact verified boundary.
