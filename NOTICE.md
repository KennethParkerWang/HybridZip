# Third-Party Notices

HybridZip is distributed under GNU GPL version 3 because it contains modified
cmix source code. See `LICENSE` and `third_party/cmix/COPYING`.

## cmix

- Project: cmix
- Source: https://github.com/byronknoll/cmix
- Revision used: `1d95fe95381a01442fceab585375cdec7c06922f`
- License: GNU GPL version 3
- Use: adapted PPMD, Online LSTM, and Match implementations
- GPLv3 section 5(a) modification notice: cmix-derived files were modified for
  HybridZip on 2026-08-20.

The exact modified files and changes are listed in
`third_party/cmix/MODIFICATIONS.md`. The complete GPLv3 text accompanies them
in `third_party/cmix/COPYING` and the repository-root `LICENSE`.

## PAQ8px

- Project: PAQ8px
- Source: https://github.com/hxim/paq8px
- Revision used: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- License: GNU GPL version 2 or later
- Use: adapted generic MatchCore implementation

The adapted files are `third_party/paq8px/match_core.h` and
`third_party/paq8px/match_core.cpp`. License and attribution are preserved in
`third_party/paq8px/LICENSE.md`; exact upstream paths and hashes are in
`third_party/paq8px/PROVENANCE.md`; modifications are described in
`third_party/paq8px/MODIFICATIONS.md`.

The project-owned backend in
`src/r2/entropy/donor_match_predictive_backend.{h,cpp}` combines
project-owned adapters around the cmix and PAQ8px donor-derived Match
components. Its integration test is
`tests/donor_match_predictive_backend_tests.cpp`; it is not a separate donor.

## Zstandard

- Project: Zstandard
- Source: https://github.com/facebook/zstd
- Revision used: `82d322c4973d9e2968d94047a40892bc6d9a9bdf`
- Selected license: BSD 3-Clause
- Use: complete single-threaded codec branch for HZ02 zstd blocks

The imported paths and configuration are recorded in
`third_party/zstd/PROVENANCE.md`. The BSD license is in
`third_party/zstd/LICENSE`.

## libsais

- Project: libsais
- Source: https://github.com/IlyaGrebnov/libsais
- Revision used: `b6e52ef33fe14f9d5c14c580d162b6fd2c27f2a8` (2.10.4)
- License: Apache License 2.0
- Use: single-threaded BWT and inverse-BWT implementation for HZ02 BwtZstd
  blocks

The unmodified source closure and Apache license are under
`third_party/libsais`; exact source hashes and the project-owned adapter
boundary are recorded in `third_party/libsais/PROVENANCE.md`.

## Kanzi-cpp

- Source: https://github.com/flanglet/kanzi-cpp
- Revision used: `66a80678f1a32bceb2d7949fbde05033d4d448e4`
- License: Apache License 2.0
- Use: `SBRT::MODE_MTF` and `RLT` in HZ02 BWT post-transform blocks

The imported closure and source hashes are in `third_party/kanzi/PROVENANCE.md`.

## XZ Utils

- Source: https://github.com/tukaani-project/xz
- Revision used: `11334a5d4d5ea3e8b2a3cbce74c1062d25cef772`
- License: BSD Zero Clause License (0BSD)
- Use: x86 BCJ transform in HZ02 X86BcjZstd blocks

## FiniteStateEntropy

- Project: FiniteStateEntropy
- Source: https://github.com/Cyan4973/FiniteStateEntropy
- Revision used: `9f30e0918f87bd835fa040d922a208d7b219e50b`
- License: BSD 2-Clause
- Use: direct FSE entropy backend for HZ02 blocks

The byte-identical `lib/` dependency closure is under `third_party/fse`.
HybridZip applies compile-time symbol prefixes so it can coexist with zstd's
private FSE copy. The license and integration record are in
`third_party/fse/LICENSE` and `third_party/fse/README.hybridzip.md`.

## 7-Zip LZMA SDK Subset

- Project: 7-Zip
- Source: https://github.com/ip7z/7zip
- Revision used: `f9d78aff31a5f2521ae7ddbdc97c4a8855808959`
- Subset license: public domain by explicit notice in every copied file
- Use: single-threaded LZMA1 encoder and decoder for HZ02 blocks

The 16 copied SDK files are byte-identical to upstream and consist of six C
translation units plus ten direct headers. Their paths and SHA-256 values are
listed in `third_party/7zip-lzma/PROVENANCE.md`; the per-file licensing rule
and upstream mixed-license policy are preserved in
`third_party/7zip-lzma/LICENSE.md` and
`third_party/7zip-lzma/LICENSE-7ZIP-SDK.txt`. No RAR, unRAR-restricted, or
default-LGPL source is included.

## Reference Arithmetic Coding

- Author: Project Nayuki
- Source: https://github.com/nayuki/Reference-arithmetic-coding
- Revision used: `ab6ee50afec04d235a4b82d17f407f0fd2b42e9a`
- License: MIT
- Use: arithmetic coder, bit streams, and frequency-table interface

The full MIT text is in `third_party/nayuki/LICENSE`.

## Nacrith-GPU

- Source: https://github.com/st4ck/Nacrith-GPU
- Revision reviewed: `ff29c42e5cfa77d7c00641880e99713644adc923`
- License: Apache License 2.0
- Use: algorithmic reference for the independently written N-gram predictor
  and adaptive probability mixer; no Python source is incorporated.
