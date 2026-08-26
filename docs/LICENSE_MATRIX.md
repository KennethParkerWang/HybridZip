# R2 Donor License Matrix

HybridZip is distributed under `GPL-3.0` because it contains modified cmix
code. The compatibility notes below apply to combining selected donor files
with this GPL-3.0 project. They are engineering gates, not legal advice.

A downloaded checkout is available for study only. Copying code requires an
exact file list, a license check at the pinned revision, retained notices, and
an updated third-party provenance record.

## Matrix

| Donor | Declared or selected license | License evidence and SHA-256 | GPL-3.0 integration boundary |
| --- | --- | --- | --- |
| paq8px | `GPL-2.0-or-later` | `compressors/context-mixing/paq8px/README.md`<br>`898F9146C9B9DEB3AB419F3B7F3F61A8E1E690931E2D074169874502035D761A` | The accepted extraction includes MatchCore, APM1, the complete 310-file `src` snapshot, scalar portability adaptations, Generic and specialist Text/x86/Image/Audio/JPEG/DEC graphs, full SSE, and the detector-only adaptation of `filter/Filters.hpp`. GPL-2.0-or-later is compatible with GPL-3.0 through the "or later" option; preserve `third_party/paq8px/{LICENSE.md,PROVENANCE.md,MODIFICATIONS.md,*_AUDIT.md,block_detection/upstream/filter}`. Optional LSTM remains outside this approval. |
| cmix | `GPL-3.0` | `compressors/context-mixing/cmix/COPYING`<br>`0B383D5A63DA644F628D99C33976EA6487ED89AAA59F0B3257992DEAC1171E6B` | Compatible and already the reason for HybridZip's GPL-3.0 distribution. The accepted PPMD, Online LSTM, and Match adaptations are listed in `third_party/cmix/MODIFICATIONS.md`; other modules are not approved by status alone. |
| 7-Zip | Mixed: `LGPL-2.1-or-later`, BSD-2-Clause, BSD-3-Clause, file-level public-domain declarations, and LGPL plus unRAR restrictions | `compressors/lz/7zip/DOC/License.txt`<br>`9AC2B4A97AB5D523965534D8B2D5868E511B39096D51FFF458AB72C38B80FCCC` | No repository-wide copying approval. The accepted 28-file LZMA1/BCJ2/PPMd7/PPMd8 subset is public domain by explicit per-file notices; exact paths and hashes are in `third_party/7zip-lzma/PROVENANCE.md`. |
| ZPAQ/libzpaq | `Unlicense/public domain` with embedded MIT divsufsort | `compressors/meta/zpaq/COPYING`<br>`927B5FEDA84F7A7F2063998B124829182967F54B954DB2C3569E8BD07958BF07` | The accepted runtime closure is `libzpaq.cpp` and `libzpaq.h`; retain the embedded MIT notice, donor source header, and `third_party/zpaq/COPYING`. |
| fumin/ctw | `BSD-3-Clause` | `compressors/ppm-ctw/LICENSE`<br>`B9A386F350DCA0BCEF67C3A0F903121D98429819D7C85386D6C520013736166E` | Compatible with GPL-3.0. The C++17 conversion covers `ctw.go` and `ac/willems/willems.go`; retain `third_party/ctw/LICENSE` and the source/conversion ledger in `third_party/ctw/PROVENANCE.md`. |
| zstd | Upstream: `BSD-3-Clause OR GPL-2.0`; selected: `BSD-3-Clause` | `compressors/lz/zstd/LICENSE`<br>`7055266497633C9025B777C78EB7235AF13922117480ED5C674677ADC381C9D8` | The complete codec branch is integrated under BSD-3-Clause. Retain the copyright, conditions, and disclaimer in source and binary distributions; imported paths are in `third_party/zstd/PROVENANCE.md`. |
| LZ4 | Repository mixed; selected `lib/` subset: `BSD-2-Clause` | `compressors/lz/lz4/lib/LICENSE`<br>`8B58C446121A109CCF32EDC094BBA3010A3D85E4EE3702950DB55E4D3E87736C` | Compatible with GPL-3.0. HybridZip copies only `lib/{lz4,lz4hc}.{c,h}` and the BSD license. GPL-2.0-or-later programs/tests are not copied. Exact hashes and the HZ41 adapter boundary are in `third_party/lz4/PROVENANCE.md`. |
| libsais | `Apache-2.0` | `compressors/bwt/libsais/LICENSE`<br>`3DDF9BE5C28FE27DAD143A5DC76EEA25222AD1DD68934A047064E56ED2FA40C5` | Compatible with GPL-3.0. The accepted closure is only `include/libsais.h` plus `src/libsais.c`; retain `third_party/libsais/LICENSE` and the copied-file hashes in `third_party/libsais/PROVENANCE.md`. |
| kanzi-cpp | `Apache-2.0` | `compressors/meta/kanzi-cpp/LICENSE`<br>`4802925085B262835797A02BFC603B04F18188856DD4501A12D482383A09125F` | Compatible with GPL-3.0. Preserve the Apache license and notices, identify modified files, and carry any upstream NOTICE obligations. The accepted closure now includes SBRT/RLT and scalar ANS files listed in `third_party/kanzi/PROVENANCE.md`. |
| FiniteStateEntropy | `BSD-2-Clause` | `entropy/fse/LICENSE`<br>`C676DF0814357087A875943355095D0EAF24E28C4EF6C0523A2C2C1B23712F66` | The byte-identical `lib/` dependency closure is integrated. Retain the copyright, conditions, and disclaimer; compile-time symbol prefixing is recorded in `third_party/fse/README.hybridzip.md`. |
| ryg_rans | `CC0-1.0` | `entropy/ryg-rans/LICENSE`<br>`518937FD5BBBDD56A3E56801CEF003997B247456BDB6E1726C8E4CB41CA41835` | Compatible. Preserve attribution and provenance even though CC0 waives copyright and related rights to the extent permitted by law. |
| FastPFOR | `Apache-2.0` | `transforms/numeric/FastPFOR/LICENSE`<br>`DC1F5D2D43C5531DFE0ACAF4E950EA5DBE3E61E1850CF0E983BDA7EFC10D6693` | Compatible. The complete source/header closure is retained at `third_party/fastpfor`; the HZ02 adapter keeps word byte order, tail data, and donor framing decoder-visible. |
| Apache Arrow | `Apache-2.0` | `transforms/numeric/apache-arrow/LICENSE.txt`<br>`23FC45DCE1769D9DDF4AAC4D6CDAF3F7F0D14FCC4D930DD0D4AFBEFA2EA3322A` | Compatible. The Parquet `DELTA_BINARY_PACKED` contract is converted into the project-owned C++17 `DeltaBinaryPackedTransform`; Arrow source remains in `KU` for study and is not compiled or distributed. See `third_party/apache-arrow/PROVENANCE.md`. |
| bGPT | `MIT` | `neural/shared/bgpt/LICENSE`<br>`7B7C34F56E9BC47CF79D6E52706ACCFE18E1BFB8DDD889459E872389FA79CF8C` | Compatible. HybridZip retains the MIT license and a generated integer posterior table derived from the pinned text checkpoint. The integration is a bounded fixed-bootstrap projection, not a complete bGPT runtime or a claim that all upstream modules were ported. |
| Language Modeling Is Compression | `Apache-2.0` | `neural/shared/lmic/LICENSE`<br>`3DDF9BE5C28FE27DAD143A5DC76EEA25222AD1DD68934A047064E56ED2FA40C5` | Compatible. HybridZip converts the donor base-2/precision-32 arithmetic-coder lifecycle into project-owned C++17 and pairs it with the existing frozen bGPT posterior. No LMIC Python source or unavailable Transformer checkpoint is distributed; the explicit archive identity is `lmic-arithmetic-frozen-bgpt-v1`. See `third_party/lmic/{PROVENANCE.md,MODIFICATIONS.md}`. |
| jax-compress | `Unlicense` | `neural/online/jax-compress/LICENSE`<br>`6B0382B16279F26FF69014300541967A356A666EB0B91B422F6862F6B7DAD17E` | Public-domain dedication permits the independently converted C++17 mode 26 backend. HybridZip carries a reduced CPU profile with exact source/profile identity metadata; it does not claim equivalence to the donor's default TPU profile. Preserve `third_party/jax-compress/{LICENSE,PROVENANCE.md}`. |
| lstm-compress | `GPL-3.0` | `neural/online/lstm-compress/COPYING`<br>`0B383D5A63DA644F628D99C33976EA6487ED89AAA59F0B3257992DEAC1171E6B` | Compatible with HybridZip's existing GPL-3.0 cmix boundary. The copied raw codec closure and project-owned HZ02 `HLC1` adapter are listed in `third_party/lstm-compress/PROVENANCE.md` and `MODIFICATIONS.md`; optional dictionary preprocessing is not copied into the archive path. |
| WavPack | `BSD-3-Clause` | `transforms/audio/wavpack/COPYING`<br>`66182C49C182998173188B0431D4DE653274C9F43D391A3EF2489C69952B1A6A` | Compatible with GPL-3.0. The accepted 24-file lossless memory pack/unpack closure is listed in `third_party/wavpack/PROVENANCE.md`; retain the BSD notice and identify the project-owned `HZW1` framing adapter. |

Evidence paths are relative to `E:/MIXER/KU/hybridzip-r2`. The SHA-256 covers
the exact evidence file at the recorded donor revision.

The two Match boundaries are exercised directly by
`tests/cmix_match_expert_tests.cpp` and `tests/paq8px_match_tests.cpp`. The
project-owned donor-match predictive backend combines adapters around those
two donor-derived components; its coding integration is exercised by
`tests/donor_match_predictive_backend_tests.cpp`. That backend is integration
evidence, not an additional donor or port-evidence record.

## 7-Zip File-Level Gate

`DOC/License.txt` defines different terms by path:

- `CPP/7zip/Compress/Rar*` is LGPL plus the unRAR restriction. These files and
  code derived from them must not enter HybridZip.
- `CPP/7zip/Compress/LzfseDecoder.cpp` and `C/ZstdDec.c` are BSD-3-Clause.
- `C/Xxh64.c` is BSD-2-Clause.
- A file is public domain only when that status is stated in the file.
- Other files without a more specific declaration are
  `LGPL-2.1-or-later` under the repository license text.

The current LZMA1, BCJ2, PPMd7, and PPMd8 ports satisfy this gate only for the
28 files listed in `third_party/7zip-lzma/PROVENANCE.md`. Fourteen C translation
units and fourteen direct headers form the accepted dependency set. Each file
carries an explicit public-domain notice. Before using BCJ or another 7-Zip
file, repeat the file-level review. A candidate-module name, `ported`
status, or `source_copy_allowed` value is not license evidence.

## Port Approval Checklist

A donor port is ready to merge only when all of these are recorded:

1. Upstream URL, exact commit, source paths, and local evidence hash.
2. License for every copied file and its dependency closure.
3. Selected license when upstream offers alternatives.
4. Required copyright, license, attribution, NOTICE, and modification text.
5. Destination paths under `third_party/` or independently written adapter
   paths under `src/`.
6. Byte-exact round-trip tests and a statement of decoder-required metadata.
7. Updated `DONORS.md`, this matrix, `NOTICE.md`, and donor provenance.

If any file has unclear, conflicting, non-commercial, field-of-use, or
reverse-engineering restrictions, stop the port until the terms are resolved.
