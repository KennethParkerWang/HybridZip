# Third-Party Notices

HybridZip is distributed under GNU GPL version 3 because it contains modified
cmix source code. See `LICENSE` and `third_party/cmix/COPYING`.

## cmix

- Project: cmix
- Source: https://github.com/byronknoll/cmix
- Revision used: `1d95fe95381a01442fceab585375cdec7c06922f`
- License: GNU GPL version 3
- Use: adapted PPMD, Online LSTM, Match, and WRT word-dictionary
  implementations
- GPLv3 section 5(a) modification notice: cmix-derived files were modified for
  HybridZip on 2026-08-20.

The exact modified files and changes are listed in
`third_party/cmix/MODIFICATIONS.md`. The complete GPLv3 text accompanies them
in `third_party/cmix/COPYING` and the repository-root `LICENSE`.

The HZ02 cmix word-dictionary candidate embeds the fixed donor
`english.dic` resource at build time and applies zstd after the reversible
dictionary transform. It does not require a KU checkout or external dictionary
file at decode time.

## PAQ8px

- Project: PAQ8px
- Source: https://github.com/hxim/paq8px
- Revision used: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- License: GNU GPL version 2 or later
- Use: adapted MatchCore, APM1, complete prediction-source snapshot,
  Generic and specialist context models, complete SSE, and decoder-visible
  block detection

The adapted files are `third_party/paq8px/match_core.{h,cpp}` and
`third_party/paq8px/apm1.{h,cpp}`. License and attribution are preserved in
`third_party/paq8px/LICENSE.md`; exact upstream paths and hashes are in
`third_party/paq8px/PROVENANCE.md`; modifications are described in
`third_party/paq8px/MODIFICATIONS.md`.

The project-owned backend in
`src/r2/entropy/donor_match_predictive_backend.{h,cpp}` combines
project-owned adapters around the cmix and PAQ8px donor-derived Match
components. Its integration test is
`tests/donor_match_predictive_backend_tests.cpp`; it is not a separate donor.

The `lstm-compress` donor raw codec closure is copied under
`third_party/lstm-compress` under GPL-3.0 and converted to the project-owned
`lstm_compress_donor_port.{h,cpp}`. HZ02 mode 23 uses the donor four-gate
online LSTM, 256-slot byte-bit model, and 32-bit range coder inside the
versioned `HLC1` payload envelope. The source closure, hashes, and documented
adaptations are recorded in `third_party/lstm-compress/{PROVENANCE.md,MODIFICATIONS.md}`.

The APM1 extraction is connected through HZ02 mode 31 / entropy 14. It is a
narrow calibration layer and does not include PAQ8px's full SSE or block
detector. The complete
RecordModel prediction closure is separately connected through HZ02 mode 32 /
entropy 15; its 52-file dependency and adapter boundary are recorded in
`third_party/paq8px/RECORD_MODEL_AUDIT.md` and `PROVENANCE.md`. The complete
scalar LinearPredictionModel/OLS/ResidualMap closure is connected through mode
33 / entropy 16 and audited in
`third_party/paq8px/LINEAR_PREDICTION_MODEL_AUDIT.md`.

The complete SimilarityModel/ContextMap2/scalar-EMA closure is separately
connected through HZ02 mode 34 / entropy 17. Its 10-file donor closure and
scalar factory adaptation are recorded in
`third_party/paq8px/SIMILARITY_MODEL_AUDIT.md` and
`third_party/paq8px/PROVENANCE.md`.

The complete APM/APM1/APMPost/SSE closure is connected after the complete
SimilarityModelPair through HZ02 mode 35 / entropy 18. Its eight donor files,
single precision-dependency adaptation, and Q31-to-Q24 boundary are recorded
in `third_party/paq8px/SSE_AUDIT.md` and `PROVENANCE.md`.

The complete non-LSTM `ContextModelGeneric` portfolio is connected before the
same full SSE through HZ02 mode 36 / entropy 19. Its 77 additional donor files
are byte-identical, while project-owned session glue replaces the donor's
global static model factory. Scope and lifecycle evidence are recorded in
`third_party/paq8px/GENERIC_MODEL_AUDIT.md` and `PROVENANCE.md`.

HZ02 mode 37 / entropy 20 adds decoder-visible PAQ8px block detection and the
donor Text, x86, Image8/24/32, Audio8/16, JPEG, and DEC Alpha specialist
graphs. The complete 310-file upstream `src` set is retained with eight
documented scalar/session/precision adaptations; the 25-file upstream filter
directory is preserved byte-for-byte beside the detector-only adapter.
Payload framing, excluded transform-only branches, and hash enforcement are
recorded in `third_party/paq8px/DETECTED_SSE_AUDIT.md`.

## ZPAQ/libzpaq

- Project: ZPAQ/libzpaq
- Source: https://mattmahoney.net/dc/zpaq.html
- Release used: zpaq 7.15 / libzpaq API 7.12
- License: Unlicense/public domain; embedded divsufsort is MIT
- Use: complete single-block ZPAQ coding path in HZ02 mode 29

The byte-identical donor closure and original notices are retained in
`third_party/zpaq`; file hashes and the adapter boundary are documented in
`third_party/zpaq/PROVENANCE.md`.

## fumin/ctw

- Project: Context Tree Weighting
- Source: https://github.com/fumin/ctw
- Revision used: `5fce9921d398dc3b720c188ebefd807dfc4f1f63`
- License: BSD 3-Clause
- Use: C++17 conversion of the CTW/KT model and Willems arithmetic coder for
  HZ02 mode 30

The BSD license is retained in `third_party/ctw/LICENSE`; exact donor hashes,
converted responsibilities, and HybridZip framing changes are documented in
`third_party/ctw/PROVENANCE.md`.

## Zstandard

- Project: Zstandard
- Source: https://github.com/facebook/zstd
- Revision used: `82d322c4973d9e2968d94047a40892bc6d9a9bdf`
- Selected license: BSD 3-Clause
- Use: complete single-threaded codec branch for HZ02 zstd blocks

The imported paths and configuration are recorded in
`third_party/zstd/PROVENANCE.md`. The BSD license is in
`third_party/zstd/LICENSE`.

## LZ4

- Project: LZ4
- Source: https://github.com/lz4/lz4
- Release/revision used: `v1.10.0` / `ebb370ca83af193212df4dcbadcc5d87bc0de2f0`
- Selected license: BSD 2-Clause for files under `lib/`
- Use: LZ4 HC block encoder and safe block decoder for HZ02 mode 39

HybridZip copies only the five-file BSD library closure under
`third_party/lz4`; GPL-2.0-or-later command-line programs and tests are not
included. Exact hashes and the project-owned `HZ41` framing boundary are in
`third_party/lz4/PROVENANCE.md`.

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
- Use: `SBRT::MODE_MTF`, `RLT` in HZ02 BWT post-transform blocks, and the
  scalar ANS range coder in HZ02 mode 40

The imported closure and source hashes are in `third_party/kanzi/PROVENANCE.md`.

## XZ Utils

- Source: https://github.com/tukaani-project/xz
- Revision used: `11334a5d4d5ea3e8b2a3cbce74c1062d25cef772`
- License: BSD Zero Clause License (0BSD)
- Use: x86 BCJ transform in HZ02 X86BcjZstd blocks

## C-Blosc2

- Source: https://github.com/Blosc/c-blosc2
- Revision used: `b17d0c3dae8d48800726a85455d9f1fdf0578b16`
- License: BSD 3-Clause
- Use: generic byte shuffle in HZ02 ShuffleZstd blocks

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

## FastPFOR

- Project: FastPFOR
- Source: https://github.com/fast-pack/FastPFOR.git
- Revision used: `2457e1ed1af35bbf7f4c509c863fa9797e637cb3`
- License: Apache License 2.0
- Use: scalar `FastPFor<8>` integer coding path for HZ02 FastPfor blocks

The Apache license, AUTHORS file, complete source/header closure, and source
identity are retained in `third_party/fastpfor`. HybridZip-owned HZ02 framing
is documented in `third_party/fastpfor/PROVENANCE.md`.

## Apache Arrow

- Project: Apache Arrow Parquet encoding
- Source: https://github.com/apache/arrow
- Revision used: `eafe3a9e620cf94683dee2347f370c35156dc965`
- License: Apache License 2.0
- Use: donor reference for the `DELTA_BINARY_PACKED` integer representation
  in HZ02 mode 42

The Apache license evidence is retained at
`E:/MIXER/KU/hybridzip-r2/transforms/numeric/apache-arrow/LICENSE.txt`.
HybridZip distributes the project-owned C++17 conversion in
`src/r2/representation/delta_binary_packed_transform.{h,cpp}`, not Arrow
source files. The conversion boundary and source hashes are recorded in
`third_party/apache-arrow/PROVENANCE.md`.

## CharLS

- Project: CharLS
- Source: https://github.com/team-charls/charls
- Revision used: `c0bae6496fa5d787fbb4698debd1e5decb40cf3a`
- License: BSD 3-Clause
- Use: complete JPEG-LS encoder/decoder for HZ02 JpegLs image-frame blocks

The byte-identical source/header closure and BSD license are retained in
`third_party/charls`. Its integration boundary is recorded in
`third_party/charls/PROVENANCE.md`.

## FLAC / libFLAC

- Project: FLAC
- Source: https://github.com/xiph/flac.git
- Revision used: `e94ff9f68b8e7dbd3e9f8b1ac18a8eca1914f181`
- License: BSD 3-Clause
- Use: scalar fixed/LPC prediction and residual primitives for HZ02
  FlacResidual 16-bit PCM blocks

The byte-identical scalar donor closure and BSD license are retained in
`third_party/flac`. Its project-owned framing boundary is recorded in
`third_party/flac/PROVENANCE.md`.

## WavPack

- Project: WavPack
- Source: https://github.com/dbry/WavPack
- Revision used: `eccf998c7acce58e18dedd354e6b025728dcf6da`
- License: BSD 3-Clause
- Use: complete lossless PCM pack/unpack closure for HZ02 mode 38

The 24 copied donor files are byte-identical to the pinned checkout. The
license, source hashes, and the project-owned `HZW1` memory framing boundary
are recorded in `third_party/wavpack/PROVENANCE.md`.

## Brotli

- Project: Brotli
- Source: https://github.com/google/brotli
- Revision used: `8e10eeb3378f6c459dbaf033ca6727e9816afccb`
- License: MIT
- Use: complete text-mode encoder/decoder and static dictionary/context path
  for HZ02 BrotliText blocks

The byte-identical C source closure and MIT license are retained in
`third_party/brotli`. The HZ02 adapter and source identity are documented in
`third_party/brotli/PROVENANCE.md`.

## ryg-rans

- Project: ryg-rans
- Source: https://github.com/rygorous/ryg_rans
- Revision used: `c9d162d996fd600315af9ae8eb89d832576cb32d`
- License: CC0-1.0
- Use: scalar byte rANS entropy backend for HZ02 Rans blocks

The byte-identical header and CC0 license are retained in `third_party/rans`.
HybridZip-owned static-model framing and validation are documented in
`third_party/rans/PROVENANCE.md`.

## 7-Zip Public-Domain Codec Subset

- Project: 7-Zip
- Source: https://github.com/ip7z/7zip
- Revision used: `f9d78aff31a5f2521ae7ddbdc97c4a8855808959`
- Subset license: public domain by explicit notice in every copied file
- Use: single-threaded LZMA1, BCJ2, PPMd7H/7z-range-coder, and
  PPMdI/carryless-range-coder paths for HZ02 blocks

The 28 copied SDK files are byte-identical to upstream and consist of fourteen
C translation units plus fourteen direct headers. Their paths and SHA-256 values are
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

## bGPT

- Project: bGPT
- Source: https://github.com/sanderwood/bgpt
- Revision used: `56b98d647b97c086bf9b3c0b840f0d662545e81c`
- Checkpoint revision: `7b3fc8b7fe0b4fec4f40dd0bdeb39b2cacf0aa96`
- License: MIT
- Use: generated fixed-bootstrap 257-context byte posterior for HZ02
  BgptSharedPrior blocks

The retained checkpoint and source checkout stay in the external KU donor
warehouse. HybridZip distributes the generated integer frequency table and
the upstream MIT license under `third_party/bgpt-shared-prior`; exact hashes,
the projection procedure, and the limitation that this is not a complete
bGPT runtime are recorded in `third_party/bgpt-shared-prior/PROVENANCE.md`.

## Language Modeling Is Compression

- Project: Language Modeling Is Compression
- Source: https://github.com/google-deepmind/language_modeling_is_compression
- Revision reviewed: `b5c8f8a63349d0a2604367d47df4a7c79db52890`
- License: Apache License 2.0
- Use: donor arithmetic-coder lifecycle and probability-normalization boundary
  for HZ02 mode 41. The project-owned C++17 port is paired with the frozen bGPT
  byte prior under the explicit identity `lmic-arithmetic-frozen-bgpt-v1`.

The upstream repository does not contain the pretrained Transformer checkpoint
required for its complete codec. HybridZip therefore does not claim a complete
LMIC Transformer reproduction and does not distribute LMIC Python source.
The exact port boundary and donor hashes are recorded in
`third_party/lmic/{PROVENANCE.md,MODIFICATIONS.md}`.

## jax-compress

- Project: jax-compress
- Source: https://github.com/byronknoll/jax-compress
- Revision used: `77adbc581eb0819a77e47c50ff6ed8ece338e60c`
- License: Unlicense
- Use: C++17 CPU-portable conversion of the decoder-synchronized online
  LSTM/Adam lifecycle for HZ02 mode 26

The included profile is intentionally smaller than the donor's default TPU
configuration. Exact architecture, update schedule, numeric boundary, archive
identity, and non-equivalence statement are retained in
`third_party/jax-compress/PROVENANCE.md`.
