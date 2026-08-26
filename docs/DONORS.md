# R2 Donor Inventory

HybridZip R2 follows a donor-first policy: when a mature implementation is
available, study and port that implementation before writing a replacement.
The authoritative research checkouts are stored under
`E:/MIXER/KU/hybridzip-r2`. They are not part of the HybridZip distribution.

This inventory governs source identity and research status. It does not grant
permission to copy code. The file-level review required before a port is in
[LICENSE_MATRIX.md](LICENSE_MATRIX.md).

## Current Inventory

| Donor | Authoritative root | Revision | Candidate modules | Status |
| --- | --- | --- | --- | --- |
| paq8px | `compressors/context-mixing/paq8px` | `29237fb44cb1995690e3eb72c6c3b1e4aede5791` | MatchCore, APM1 calibration, complete prediction-source snapshot, Generic and specialist model graphs, full SSE, and decoder-visible block detection | ported |
| cmix | `compressors/context-mixing/cmix` | `1d95fe95381a01442fceab585375cdec7c06922f` | PPMd, match, WRT word dictionary, byte model, mixer/SSE, online LSTM | ported |
| 7-Zip | `compressors/lz/7zip` | `f9d78aff31a5f2521ae7ddbdc97c4a8855808959` | LZMA match finder/parser, PPMd7/8, BCJ/BCJ2 | ported |
| ZPAQ/libzpaq | `compressors/meta/zpaq` | `zpaq-7.15` release snapshot | complete ZPAQ block compressor, ZPAQL context mixing, LZ77/BWT preprocessing | ported |
| fumin/ctw | `compressors/ppm-ctw` | `5fce9921d398dc3b720c188ebefd807dfc4f1f63` | CTW context tree, KT estimator, Willems arithmetic coder | ported |
| zstd | `compressors/lz/zstd` | `82d322c4973d9e2968d94047a40892bc6d9a9bdf` | complete codec, matchers, optimal parser, long-distance matching, FSE/Huffman, stored decision | ported |
| LZ4 | `compressors/lz/lz4` | `ebb370ca83af193212df4dcbadcc5d87bc0de2f0` | block codec, HC parser, safe block decoder | ported |
| libsais | `compressors/bwt/libsais` | `b6e52ef33fe14f9d5c14c580d162b6fd2c27f2a8` | suffix array, BWT, inverse BWT | ported |
| kanzi-cpp | `compressors/meta/kanzi-cpp` | `66a80678f1a32bceb2d7949fbde05033d4d448e4` | transform portfolio, BWT, context mixing, ANS, content classification | ported |
| FiniteStateEntropy | `entropy/fse` | `9f30e0918f87bd835fa040d922a208d7b219e50b` | FSE, Huff0 | ported |
| ryg_rans | `entropy/ryg-rans` | `c9d162d996fd600315af9ae8eb89d832576cb32d` | scalar rANS, rANS64, SIMD rANS | ported |
| lstm-compress | `neural/online/lstm-compress` | `bbbbff0e9bc9a2052754068c1867e0e84344cabc` | standalone online LSTM, 256-slot byte-bit model, and donor range coder | ported; GPL-3.0 source closure copied |
| jax-compress | `neural/online/jax-compress` | `77adbc581eb0819a77e47c50ff6ed8ece338e60c` | online LSTM lifecycle, Adam test-time updates, periodic retraining, integer arithmetic-coder boundary | ported |
| bGPT | `neural/shared/bgpt` | `56b98d647b97c086bf9b3c0b840f0d662545e81c` | pretrained byte posterior, patch/byte decoders, shared-model profile | ported: fixed-bootstrap posterior projection only |
| Language Modeling Is Compression | `neural/shared/lmic` | `b5c8f8a63349d0a2604367d47df4a7c79db52890` | language-model posterior adapter, integer arithmetic coder, probability normalization | ported: arithmetic-coder lifecycle paired with frozen bGPT posterior; Transformer checkpoint unavailable |
| FastPFOR | `transforms/numeric/FastPFOR` | `2457e1ed1af35bbf7f4c509c863fa9797e637cb3` | scalar FastPFor, NewPFor, BinaryPacking | ported |
| CharLS | `transforms/image/charls` | `c0bae6496fa5d787fbb4698debd1e5decb40cf3a` | JPEG-LS scan encoder/decoder, LOCO-I spatial predictor | ported |
| FLAC | `transforms/audio/flac` | `e94ff9f68b8e7dbd3e9f8b1ac18a8eca1914f181` | fixed prediction, LPC residual calculation, Rice residual coding | ported |
| WavPack | `transforms/audio/wavpack` | `eccf998c7acce58e18dedd354e6b025728dcf6da` | lossless PCM packer, unpacker, decorrelation and entropy coding | ported |
| Brotli | `compressors/text/brotli` | `8e10eeb3378f6c459dbaf033ca6727e9816afccb` | text-mode encoder, static dictionary/contexts, decoder | ported |
| Apache Arrow | `transforms/numeric/apache-arrow` | `eafe3a9e620cf94683dee2347f370c35156dc965` | Parquet `DELTA_BINARY_PACKED` encoder/decoder and bit-packed integer residuals | ported |

Research-only donor checkouts are retained outside the closed twenty-one-root
runtime inventory. Apache Parquet Format at
`transforms/numeric/parquet-format`, revision
`24102ed5c56e51b610a4897e5f79e76e43732d1d` (Apache-2.0), is `downloaded`.
Its `BYTE_STREAM_SPLIT` specification informed the project-owned
16/32-byte `RecordTransposeZstd` contract; no Parquet source, container, or
Thrift parser is incorporated in HybridZip.

Paths in the table are relative to `E:/MIXER/KU/hybridzip-r2`. The fixed
inventory contains twenty-one donor roots. CharLS, Brotli, and jax-compress are codeload archive
donors: their retained tarball SHA-256 values are validated in place rather
than claiming nonexistent Git checkouts. An empty category directory, such as
`compressors/ppm-ctw` is the pinned fumin/ctw Git checkout and has a required
manifest.

## Provenance Contract

Every donor root must contain a UTF-8 `DONOR.json` with these fields:

```json
{
  "name": "upstream project name",
  "url": "https://authoritative.example/repository",
  "revision": "40-character Git commit",
  "license": "license identifier or exact mixed-license declaration",
  "language": "implementation language",
  "candidate_modules": ["module-name"],
  "download_date": "YYYY-MM-DD",
  "status": "downloaded"
}
```

Allowed status values are:

| Status | Meaning |
| --- | --- |
| `downloaded` | The pinned checkout and license evidence exist locally. |
| `studied` | The candidate modules and their integration boundary were reviewed. |
| `ported` | At least one accepted subset is present in HybridZip with notices and tests. |
| `rejected` | The candidate was evaluated and rejected; the reason belongs in the central provenance notes or experiment record. |

The compact root manifest mirrors the required identity fields in
`provenance/<donor>.json`. The central record additionally stores the local
path, branch, evidence path and SHA-256, checkout size, and research notes.
`provenance/warehouse.json` is the closed inventory and pins the architecture
source hash. For zstd, the root manifest records the selected
`BSD-3-Clause` option while the central record also preserves the upstream
dual-license declaration.

`ported` applies only to the accepted subset named below. It does not approve
every candidate module or override a file-level license boundary.

## Integrated Subsets

| Donor | Accepted subset | Destination and evidence |
| --- | --- | --- |
| paq8px | Adapted generic MatchCore, APM1 calibration, complete 310-file prediction-source snapshot, Generic and specialist Text/x86/Image/Audio/JPEG/DEC graphs, full SSE, and decoder-visible block detection | `third_party/paq8px`; source identities and adaptation boundaries in `PROVENANCE.md`, `MODIFICATIONS.md`, and the six `*_AUDIT.md` records including `DETECTED_SSE_AUDIT.md` |
| cmix | Adapted PPMD, Online LSTM, Match, and build-embedded WRT word dictionary implementation | `third_party/cmix`; source evidence and modified-file ledger in `third_party/cmix/MODIFICATIONS.md` |
| zstd | Complete single-threaded codec branch under the selected BSD-3-Clause option | `third_party/zstd`; imported paths in `third_party/zstd/PROVENANCE.md` |
| LZ4 | Byte-identical BSD-2-Clause block codec, HC parser, and safe decoder closure | `third_party/lz4`; source hashes and HZ41 adapter boundary in `third_party/lz4/PROVENANCE.md` |
| libsais | Unmodified single-threaded BWT/inverse-BWT C source with a project-owned primary-index adapter | `third_party/libsais`; source hashes in `third_party/libsais/PROVENANCE.md` |
| kanzi-cpp | SBRT MTF, RLT post-BWT transform, and scalar ANS entropy closure | `third_party/kanzi/PROVENANCE.md` |
| FiniteStateEntropy | Byte-identical `lib/` dependency closure for the direct FSE block backend | `third_party/fse`; integration record in `third_party/fse/README.hybridzip.md` |
| FastPFOR | Scalar `FastPFor<8>` bit-packing closure with HZ02 little-endian word and tail framing | `third_party/fastpfor`; source identity and adapter boundary in `third_party/fastpfor/PROVENANCE.md` |
| Apache Arrow | Parquet `DELTA_BINARY_PACKED` encoder/decoder contract converted into a typed C++17 representation | `src/r2/representation/delta_binary_packed_transform.{h,cpp}`; source identity and adapter boundary in `third_party/apache-arrow/PROVENANCE.md` |
| ryg_rans | Scalar byte rANS with a decoder-visible normalized static byte model | `third_party/rans`; source identity and adapter boundary in `third_party/rans/PROVENANCE.md` |
| CharLS | Complete JPEG-LS encoder/decoder closure with an 8-bit one-component HZ02 image frame | `third_party/charls`; source identity and adapter boundary in `third_party/charls/PROVENANCE.md` |
| FLAC | Scalar libFLAC fixed/LPC residual closure with HZ02 16-bit PCM and Rice framing | `third_party/flac`; source identity and adapter boundary in `third_party/flac/PROVENANCE.md` |
| WavPack | Complete lossless PCM pack/unpack closure with decoder-visible PCM profile and raw-tail framing | `third_party/wavpack`; source identity and HZ02 adapter boundary in `third_party/wavpack/PROVENANCE.md` |
| Brotli | Complete text-mode codec with static dictionary/context support and strict HZ02 stream consumption | `third_party/brotli`; source identity and adapter boundary in `third_party/brotli/PROVENANCE.md` |
| 7-Zip | Public-domain single-threaded LZMA1, BCJ2, complete PPMd7H plus 7z range coder, and complete PPMdI plus carryless range coder, together with a direct `LzFind` binary-tree greedy parse candidate | `third_party/7zip-lzma`; paths and hashes in `third_party/7zip-lzma/PROVENANCE.md`, adapter boundaries in `third_party/7zip-lzma/MODIFICATIONS.md` |
| ZPAQ/libzpaq | Byte-identical complete `libzpaq.cpp` and API header, compiled without JIT and called through the donor single-block interface | `third_party/zpaq`; release identity and file hashes in `third_party/zpaq/PROVENANCE.md`, HZQ1 adapter in `src/r2/entropy/zpaq_backend.{h,cpp}` |
| fumin/ctw | Complete CTW/KT update-revert lifecycle and matching Willems finite-precision arithmetic coder converted from Go to C++17 | `src/r2/entropy/ctw_backend.{h,cpp}`; source identity and conversion boundary in `third_party/ctw/PROVENANCE.md` |
| bGPT | Quantized 257-context byte posterior projected reproducibly from the pinned text checkpoint under a fixed bootstrap | `third_party/bgpt-shared-prior`; checkpoint identity, projection contract, and limitations in `third_party/bgpt-shared-prior/PROVENANCE.md` and `projection.json` |
| jax-compress | CPU-portable decoder-synchronized online LSTM/Adam lifecycle with periodic history replay | `src/r2/entropy/jax_compress_portable_backend.{h,cpp}` and `third_party/jax-compress`; exact reduced profile, source identity, and compatibility boundary in `third_party/jax-compress/PROVENANCE.md` |

The fourteen compiled 7-Zip C translation units cover allocation/CPU support,
LZMA1, BCJ2, PPMd7H with the PPMd7z range coder, and PPMdI with its carryless
range coder. Every one of the 28 copied files has an explicit public-domain
notice and is byte-identical to the pinned checkout. No RAR, unRAR-restricted,
or default-LGPL file is included.

The direct FSE sources are also byte-identical to the pinned checkout.
HybridZip applies `HZFSE_`, `HZHIST_`, and `HZHUF_` symbol prefixes at compile
time so this donor can coexist with zstd's private FSE copy.

## Handling Rules

1. Keep upstream source at the recorded Git revision. The only project-owned
   file added to a donor root is `DONOR.json`.
2. Do not develop inside a donor checkout. Copy an approved, license-reviewed
   dependency closure into `third_party/` and record every copied file.
3. Record modifications, retain copyright and license notices, and update
   `NOTICE.md` plus donor-specific provenance before distributing a port.
4. Treat `candidate_modules` as research leads, not as a blanket copying
   approval. A repository-level license does not override file-level terms.
5. Include every decoder-visible choice and every side-data byte in the R2
   archive contract and measurements. Never route by benchmark filename,
   path, or test-case identity.
6. Keep HZ01 and PROFILE_V1 as regression targets. Donor experiments may add
   R2 modes but must not silently change the V1 bitstream contract.

Apart from the project-owned `DONOR.json`, the cmix checkout reports one
CRLF-only working-tree difference in `src/enwik9-preproc/main.cpp`;
`git diff --ignore-space-at-eol` is clean. The recorded commit remains the
source identity, and donor source must not be normalized as part of HybridZip
work.

## Validation

From the HybridZip repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/validate_r2_donors.ps1
```

The validator checks the closed twenty-one-donor inventory, manifest schema and
status, seventeen Git HEAD/origin identities plus the
pinned CharLS, Brotli, and jax-compress codeload archives, license evidence paths and SHA-256
values, eighteen port-evidence records, and the exact GPL-3.0 declaration for
cmix.
