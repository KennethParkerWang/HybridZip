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
| paq8px | `GPL-2.0-or-later` | `compressors/context-mixing/paq8px/README.md`<br>`898F9146C9B9DEB3AB419F3B7F3F61A8E1E690931E2D074169874502035D761A` | The modified Match extraction is limited to `third_party/paq8px/match_core.{h,cpp}`. GPL-2.0-or-later is compatible with GPL-3.0 through the "or later" option; preserve `third_party/paq8px/{LICENSE.md,PROVENANCE.md,MODIFICATIONS.md}`. No other PAQ8px module is approved by status alone. |
| cmix | `GPL-3.0` | `compressors/context-mixing/cmix/COPYING`<br>`0B383D5A63DA644F628D99C33976EA6487ED89AAA59F0B3257992DEAC1171E6B` | Compatible and already the reason for HybridZip's GPL-3.0 distribution. The accepted PPMD, Online LSTM, and Match adaptations are listed in `third_party/cmix/MODIFICATIONS.md`; other modules are not approved by status alone. |
| 7-Zip | Mixed: `LGPL-2.1-or-later`, BSD-2-Clause, BSD-3-Clause, file-level public-domain declarations, and LGPL plus unRAR restrictions | `compressors/lz/7zip/DOC/License.txt`<br>`9AC2B4A97AB5D523965534D8B2D5868E511B39096D51FFF458AB72C38B80FCCC` | No repository-wide copying approval. The accepted 16-file LZMA SDK subset is public domain by explicit per-file notices; exact paths and hashes are in `third_party/7zip-lzma/PROVENANCE.md`. |
| zstd | Upstream: `BSD-3-Clause OR GPL-2.0`; selected: `BSD-3-Clause` | `compressors/lz/zstd/LICENSE`<br>`7055266497633C9025B777C78EB7235AF13922117480ED5C674677ADC381C9D8` | The complete codec branch is integrated under BSD-3-Clause. Retain the copyright, conditions, and disclaimer in source and binary distributions; imported paths are in `third_party/zstd/PROVENANCE.md`. |
| libsais | `Apache-2.0` | `compressors/bwt/libsais/LICENSE`<br>`3DDF9BE5C28FE27DAD143A5DC76EEA25222AD1DD68934A047064E56ED2FA40C5` | Compatible with GPL-3.0. The accepted closure is only `include/libsais.h` plus `src/libsais.c`; retain `third_party/libsais/LICENSE` and the copied-file hashes in `third_party/libsais/PROVENANCE.md`. |
| kanzi-cpp | `Apache-2.0` | `compressors/meta/kanzi-cpp/LICENSE`<br>`4802925085B262835797A02BFC603B04F18188856DD4501A12D482383A09125F` | Compatible with GPL-3.0. Preserve the Apache license and notices, identify modified files, and carry any upstream NOTICE obligations. |
| FiniteStateEntropy | `BSD-2-Clause` | `entropy/fse/LICENSE`<br>`C676DF0814357087A875943355095D0EAF24E28C4EF6C0523A2C2C1B23712F66` | The byte-identical `lib/` dependency closure is integrated. Retain the copyright, conditions, and disclaimer; compile-time symbol prefixing is recorded in `third_party/fse/README.hybridzip.md`. |
| ryg_rans | `CC0-1.0` | `entropy/ryg-rans/LICENSE`<br>`518937FD5BBBDD56A3E56801CEF003997B247456BDB6E1726C8E4CB41CA41835` | Compatible. Preserve attribution and provenance even though CC0 waives copyright and related rights to the extent permitted by law. |

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

The current LZMA1 port satisfies this gate only for the 16 files listed in
`third_party/7zip-lzma/PROVENANCE.md`. Its six C translation units are
`Alloc.c`, `CpuArch.c`, `LzFind.c`, `LzFindOpt.c`, `LzmaDec.c`, and
`LzmaEnc.c`; ten direct headers complete the dependency set. Each file carries
an explicit public-domain notice. Before using PPMd7/8, BCJ, BCJ2, or another
7-Zip file, repeat the file-level review. A candidate-module name, `ported`
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
