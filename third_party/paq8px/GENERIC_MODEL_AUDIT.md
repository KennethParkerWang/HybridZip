# PAQ8px Generic context-model dependency audit

## Source identity

- Upstream: `https://github.com/hxim/paq8px`
- Revision: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- License: `GPL-2.0-or-later`
- Authoritative checkout:
  `E:/MIXER/KU/hybridzip-r2/compressors/context-mixing/paq8px`
- Vendored destination: `third_party/paq8px/record_model`
- Donor orchestration source: `src/model/ContextModelGeneric.cpp`
- Orchestration source SHA-256:
  `F767284C03C39A72F4F0DF9360445889D4F94A73908736A3D430F4BAB667BED6`

## Accepted graph

Mode 36 retains the complete non-LSTM donor `ContextModelGeneric` graph:

1. `NormalModel` and its post contexts;
2. `MatchModel`;
3. `SparseMatchModel`, `SparseBitModel`, and `SparseModel`;
4. `ChartModel`, `RecordModel`, and `CharGroupModel`;
5. `TextModel` and binary-profile `WordModel`;
6. `IndirectModel`, `DmcForest`, `NestModel`, and `XMLModel`;
7. `LinearPredictionModel`, the slow/fast `SimilarityModelPair`, and the
   last-in-order `ExeModel`;
8. the complete `SSE` object after the combined scalar Mixer posterior.

Optional PAQ8px LSTM is disabled for this fixed profile. Every non-LSTM model
called by the donor Generic path is present and mixed in donor order. The
profile fixes `BlockType::DEFAULT`; decoder-visible specialist block detection
and transformations remain a separate integration branch.

## Source closure

The previously audited RecordModel, LinearPredictionModel, SimilarityModel,
and SSE closures are reused. The Generic expansion adds exactly 77 files and
271,094 upstream bytes, all byte-identical to the pinned checkout. They cover:

- 7 file-support files used by the dormant training/embedding API retained by
  the complete TextModel compilation unit;
- 8 common map/string/match-storage headers and implementations;
- 36 model files for Normal, Match, sparse, chart, character, DMC, nesting,
  XML, word, indirect, and x86/x64 models;
- 26 text/language/stemmer/word files required by the complete TextModel.

`tools/validate_r2_donors.ps1` contains the exact 77-path allowlist. For each
path it requires both the vendored file and pinned upstream source, then
compares their SHA-256 values. Existing adapted files are not overwritten:
`Shared.cpp`, `OLS_factory.cpp`, `SimilarityEmaFunctionsFactory.{hpp,cpp}`,
and `APMPost.cpp` retain their separately audited adaptations.

## Session lifecycle adaptation

Donor `Models.cpp` is not copied as runtime code because every accessor uses a
function-local static. Those objects bind the first `Shared*` and would carry
adaptive state across later HybridZip blocks or codec sessions.

The project-owned `paq8px_generic_sse_backend.{h,cpp}` instead constructs one
independent object graph for every encoder or decoder block. It preserves the
donor mix order, memory multipliers, scalar Mixer dimensions, scale factor
`980, 90`, level 1, one MiB history, invalid match byte 256, and first-byte
block-position convention. No model state is serialized or shared implicitly.

Each bit follows:

```text
17-model ContextModelGeneric graph -> Mixer_Scalar::p (Q12)
-> full SSE::p (Q31) -> deterministic Q24 quantization
-> HZ02 arithmetic coder -> Shared::update(p24 << 7)
```

The archive identifiers are mode 36, raw transform 0, entropy 19, and CLI
`--r2-mode=paq8px-generic-sse`. The payload bound is
`4 * uncompressed_size + 64`; the outer block CRC32 remains mandatory.

## Verification boundary

Release `hybridzip` and `hz_r2_codec_tests` compile and link. The test
executable remains compile-only under the current minimal-test rule. The
single forced 1 KiB encode/decode smoke produced a 483-byte archive with a
423-byte payload and decoded byte-exactly. Its tuple is `36/0/19`; evidence is
in `results/smoke/r2-paq8px-generic-sse-1k-20260821/verification.json`.
Auto, D40, CTest, batches, and larger blocks were not run.
