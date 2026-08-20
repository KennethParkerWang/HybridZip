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
| paq8px | `compressors/context-mixing/paq8px` | `29237fb44cb1995690e3eb72c6c3b1e4aede5791` | context map, match, record, linear prediction, similarity, SSE/APM, block detection | ported |
| cmix | `compressors/context-mixing/cmix` | `1d95fe95381a01442fceab585375cdec7c06922f` | PPMd, match, byte model, mixer/SSE, online LSTM | ported |
| 7-Zip | `compressors/lz/7zip` | `f9d78aff31a5f2521ae7ddbdc97c4a8855808959` | LZMA match finder/parser, PPMd7/8, BCJ/BCJ2 | ported |
| zstd | `compressors/lz/zstd` | `82d322c4973d9e2968d94047a40892bc6d9a9bdf` | complete codec, matchers, optimal parser, long-distance matching, FSE/Huffman, stored decision | ported |
| libsais | `compressors/bwt/libsais` | `b6e52ef33fe14f9d5c14c580d162b6fd2c27f2a8` | suffix array, BWT, inverse BWT | ported |
| kanzi-cpp | `compressors/meta/kanzi-cpp` | `66a80678f1a32bceb2d7949fbde05033d4d448e4` | transform portfolio, BWT, context mixing, ANS, content classification | ported |
| FiniteStateEntropy | `entropy/fse` | `9f30e0918f87bd835fa040d922a208d7b219e50b` | FSE, Huff0 | ported |
| ryg_rans | `entropy/ryg-rans` | `c9d162d996fd600315af9ae8eb89d832576cb32d` | scalar rANS, rANS64, SIMD rANS | downloaded |

Paths in the table are relative to `E:/MIXER/KU/hybridzip-r2`. The fixed
inventory contains eight donor roots. An empty category directory, such as
`compressors/ppm-ctw`, is not a donor root and does not require a manifest.

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
| paq8px | Adapted generic MatchCore with the order-9/7/5 lookup and MatchInfo lifecycle | `third_party/paq8px`; source hashes and adaptation boundary in `third_party/paq8px/PROVENANCE.md` and `third_party/paq8px/MODIFICATIONS.md` |
| cmix | Adapted PPMD, Online LSTM, and Match implementations | `third_party/cmix`; source evidence and modified-file ledger in `third_party/cmix/MODIFICATIONS.md` |
| zstd | Complete single-threaded codec branch under the selected BSD-3-Clause option | `third_party/zstd`; imported paths in `third_party/zstd/PROVENANCE.md` |
| libsais | Unmodified single-threaded BWT/inverse-BWT C source with a project-owned primary-index adapter | `third_party/libsais`; source hashes in `third_party/libsais/PROVENANCE.md` |
| kanzi-cpp | SBRT MTF post-BWT transform closure | `third_party/kanzi/PROVENANCE.md` |
| FiniteStateEntropy | Byte-identical `lib/` dependency closure for the direct FSE block backend | `third_party/fse`; integration record in `third_party/fse/README.hybridzip.md` |
| 7-Zip | Public-domain single-threaded LZMA1 SDK subset, complete LZMA backend, and direct `LzFind` binary-tree greedy parse candidate | `third_party/7zip-lzma`; paths and hashes in `third_party/7zip-lzma/PROVENANCE.md`, adapter boundary in `third_party/7zip-lzma/MODIFICATIONS.md` |

The six compiled 7-Zip donor files are `C/Alloc.c`, `C/CpuArch.c`,
`C/LzFind.c`, `C/LzFindOpt.c`, `C/LzmaDec.c`, and `C/LzmaEnc.c`. Every one of
the 16 copied SDK files has an explicit public-domain notice and is
byte-identical to the pinned checkout. No RAR, unRAR-restricted, or
default-LGPL file is included.

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

The validator checks the closed eight-donor inventory, manifest schema and
ported status, central/root agreement, Git HEAD and origin, license evidence
paths and SHA-256 values, five port-evidence records, and the exact GPL-3.0
declaration for cmix.
