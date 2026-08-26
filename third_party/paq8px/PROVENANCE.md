# PAQ8px MatchCore and APM1 provenance

- Upstream: https://github.com/hxim/paq8px
- Revision: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- Revision date: `2026-06-25T21:18:53+02:00`
- Acquisition date: `2026-08-20` (Asia/Shanghai)
- Authoritative local checkout:
  `E:\MIXER\KU\hybridzip-r2\compressors\context-mixing\paq8px`
- License evidence: upstream `README.md`, `GPL-2.0-or-later`; preserved in
  `LICENSE.md`

The extraction was made from the following files at the fixed revision.
Hashes are SHA-256 of the complete upstream files, before adaptation.

| Upstream path | Role in extraction | SHA-256 |
| --- | --- | --- |
| `src/model/MatchModel.cpp` | 9/7/5 lookup order, candidate discovery, four-candidate cap, deduplication | `46ED3F8AF07B881231FAEDBDECC6605C7370D26B9310283AD18CC27DEAE60224` |
| `src/model/MatchModel.hpp` | constants and candidate/table layout | `1C675591DAF960BF86BC018F8449FFA614E851AB4B43D7D4996956E50EB80AE1` |
| `src/model/MatchInfo.cpp` | extension, mismatch, delta, recovery, saturation, and priority lifecycle | `4DF69305BA6B7AD7BC00EAC3F3BE773A7AF65C363678FAB9D477AFDE486B912E` |
| `src/model/MatchInfo.hpp` | match state fields and operations | `A63207569D7F2DAD30DB6DB6BC396ED09F3642E01143DA0D2BBDF186FE7C1817` |
| `src/HashElementForMatchPositions.hpp` | three recent positions per hash bucket | `281FC940E290F858CB421623D612F1263BB5E6B7448A534688F1B8EF21B35581` |
| `src/Hash.hpp` | `PHI64` and high-bit `finalize64` range reduction | `F48DEA5CC073EBB66835D527BC6281FCF769130B9D85C04B6CDF5CED8AAC20E2` |
| `src/model/NormalModel.cpp` | generic order-hash recurrence | `F3633CCC11FA3F46E45825D484BD63DAAD515573AC360E7A311176198B5872D5` |
| `src/model/NormalModel.hpp` | order-hash scope and update contract | `883B99E0303A3F1DA2F2EFD091DD66EB28E57FD42F39166DAD1EFE7C37FDE26D` |
| `src/BlockType.hpp` | generic `DEFAULT = 0` and `BlockType::Count = 30` constants | `9D759F154ADD53F0D18FAAD4D6B7D22B756F2C1E44BAA1EEE14ED219FE21F0BE` |
| `README.md` | copyright and GPL-2.0-or-later evidence | `898F9146C9B9DEB3AB419F3B7F3F61A8E1E690931E2D074169874502035D761A` |
| `src/APM1.hpp` | APM1 table layout, interpolation and update contract | `0FD558B5FA78CC99DC6DD7FB76BC2C5E6584B764EEDFE4D92C42CDDD553B7C90` |
| `src/APM1.cpp` | APM1 initialization, interpolation and adaptive update | `2EC95345E787B5F2B09666307A4FBC8C976C045D159B8C84440B3C5726A725ED` |
| `src/Squash.hpp` | donor squash interface | `C3E7964CD0F0255F156FE7593746076D1CC1C94503474A7D9419360A07817BD8` |
| `src/Squash.cpp` | donor float squash table semantics | `FB203752FB935C32894EB9128EC0C452BCBC0C85BD05F70F92C96681B3AE2B69` |
| `src/Stretch.hpp` | donor stretch interface | `C69D465DAC314B4736CD1257EAA3344713E9EFF7EB5B8246860471DFD1922D15` |
| `src/Stretch.cpp` | donor float stretch table semantics | `593E0EB1CA5445B66BC1473C05B229869A205B9314E1DED87C94E74A4F84CECD` |

`match_core.h` and `match_core.cpp` are modified extractions, not byte-identical
copies. `MODIFICATIONS.md` records the adaptation boundary. No PAQ8px
`Shared`, `Mixer`, context maps, stationary maps, indirect contexts, full SSE,
block detection, or special block models are included in the MatchCore
extraction.

## HybridZip port evidence

The modified extraction is `match_core.h` and `match_core.cpp`; its behavioral
boundary is recorded in `MODIFICATIONS.md`. Project-owned integration is in
`src/r2/match/paq8px_match_service.{h,cpp}` and
`src/r2/experts/paq8px_match_expert.{h,cpp}`. Donor-golden lookup, candidate,
recovery, saturation, allocation, reset, cache, and adapter behavior is
covered by `tests/paq8px_match_tests.cpp`.

The project-owned `src/r2/entropy/donor_match_predictive_backend.{h,cpp}` also
consumes the PAQ8px Match evidence alongside the separately recorded cmix
Match posterior. Its encode/decode integration is covered by
`tests/donor_match_predictive_backend_tests.cpp`; this backend is not another
donor extraction.

## APM1 extraction and integration

The accepted APM1 subset is the six APM1/Squash/Stretch source files listed
above. `third_party/paq8px/apm1.{h,cpp}` preserves the donor's 33-point
probability interpolation, float table construction, and adaptive update rule
while replacing `Shared` and `UpdateBroadcaster` with explicit prediction and
bit-update calls. The project-owned
`src/r2/entropy/paq8px_apm_backend.{h,cpp}` applies it after the existing
decoder-synchronised V1 + cmix/PAQ8px Match posterior and exposes HZ02 mode 31
with entropy 14. The APM1 extraction itself does not claim PAQ8px's complete
SSE or block-detection models; Record, Linear Prediction, and Similarity are
separately governed branches documented below.

## RecordModel extraction and integration

The accepted RecordModel branch is the complete donor prediction graph at the
same revision. It is copied into `third_party/paq8px/record_model` as a 52-file
closure (117,508 bytes after the compile-discovered `BitCount` dependency),
including `model/RecordModel.{hpp,cpp}`, all ContextMap/StationaryMap/
SmallStationaryContextMap/IndirectMap/IndirectContext implementations,
`Shared`, state/hash/storage support, `Mixer`, `Mixer_Scalar`, and update
broadcasting. The direct RecordModel source hashes are recorded in
`RECORD_MODEL_AUDIT.md`.

The project-owned `src/r2/entropy/paq8px_record_model_backend.{h,cpp}` connects
the donor graph to HZ02 mode 32 / entropy 15. It replaces only top-level PAQ8px
file I/O, block detection, and arithmetic coding: `Shared::update` keeps the
donor 31-bit loss scaling and update order, while HZ02 supplies the binary
arithmetic bit stream. The adapter fixes `DEFAULT` block type, uses the donor
invalid Match expected-byte value 256, and converts donor Q12 probabilities to
HZ02 Q24. This is a complete RecordModel prediction port, but not a claim that
PAQ8px's full Match/SSE or block-detection portfolio was copied.

## LinearPredictionModel extraction and integration

The accepted addition is the complete 13-file scalar LinearPredictionModel
closure: `model/LinearPredictionModel`, `ResidualMap`, `Clz`, `OLS`,
`OLS_factory`, and the float/double scalar OLS implementations. It reuses the
already accepted Shared/Mixer/DivisionTable substrate. The direct model hashes
are `7D23FE1B17378F343770A0ED53667EE73A5D00277A17E5C9559269825203653F`
for the header and
`E4EB8F6522C6775B91E63DA074AB5FB628826DEBCE048362BB2BF74FF5705F36`
for the implementation; all hashes and roles are in
`LINEAR_PREDICTION_MODEL_AUDIT.md`.

Twelve files are byte-identical. `OLS_factory.cpp` is adapted from upstream
SHA-256 `9C622D3EB47CAD7478B5F9D8A6740C0FD8EC027C57ABEE001F68856C17CFB492`
to vendored SHA-256
`9D7577FCBE76F4C50F51B161ECF873E73D19AF16DCED2CE76B0CA6757D0CFC38`:
it preserves both public factory functions while removing CPU/SSE3 dispatch
and always constructing donor scalar implementations.

The project-owned `src/r2/entropy/paq8px_linear_prediction_backend.{h,cpp}`
connects the graph to HZ02 mode 33 / entropy 16. It supplies the donor Generic
bias and scale factor, fixes one outer Mixer context, uses the HZ02 binary
arithmetic stream, and preserves decoder-synchronised donor updates. This is a
complete standalone LinearPredictionModel coding path, not PAQ8px's complete
Generic portfolio or full SSE/block-detection stack.

## SimilarityModel extraction and integration

The accepted SimilarityModel branch is the complete 10-file donor addition
(`ContextMap2`, `SimilarityModelPair`, two `SimilarityModel` instances, and
scalar EMA functions) at the same pinned revision. It retains the slow/fast
EMA pair, 16+2 ResidualMap contexts, 8 ContextMap2 contexts with run statistics
and byte history, donor mixer contexts, and scalar update ordering. The exact
file hashes, 43,081-byte closure, and adaptation boundary are recorded in
`SIMILARITY_MODEL_AUDIT.md`.

The two EMA factory files remove SIMD includes and runtime dispatch so the HZ02
profile always selects the donor scalar EMA implementation; the other eight
added files are byte-identical. The project-owned
`src/r2/entropy/paq8px_similarity_backend.{h,cpp}` connects the graph to HZ02
mode 34 / entropy 17 with donor level 1, 768-byte maximum match distance,
one MiB history, scale factor 980/90, and Q12-to-Q24 conversion. Full PAQ8px
SSE and block detection remain outside this accepted subset.

## Full SSE extraction and integration

The accepted full SSE addition is the complete eight-file donor closure
`APM.{hpp,cpp}`, `APM1.{hpp,cpp}`, `APMPost.{hpp,cpp}`, and `SSE.{hpp,cpp}`.
It retains every Text/Image/Audio/JPEG/DEC/x86_64/Generic calibration table and
dispatch branch. Seven files are byte-identical; `APMPost.cpp` only replaces
the removed top-level arithmetic-coder include with its fixed 31-bit precision
constant. Exact upstream and vendored hashes are in `SSE_AUDIT.md`.

The project-owned
`src/r2/entropy/paq8px_similarity_sse_backend.{h,cpp}` connects the complete
SimilarityModelPair to the complete SSE as HZ02 mode 35 / entropy 18. SSE's
Q31 output is deterministically shifted and clamped to HZ02 Q24, and the same
quantized probability is shifted back to Q31 for decoder-synchronised donor
updates. The first profile fixes `DEFAULT` block type and therefore executes
the Generic SSE branch; decoder-visible PAQ8px block detection remains a later
integration task.

## Generic context-model extraction and integration

The accepted Generic addition completes PAQ8px's non-LSTM
`ContextModelGeneric` prediction graph and follows it with the already
accepted full SSE. It adds 77 byte-identical donor files totaling 271,094
bytes for Normal/Match/sparse/chart/character/text/word/indirect/DMC/nesting/
XML/x86 models and their compile dependencies. Exact scope and lifecycle
evidence are in `GENERIC_MODEL_AUDIT.md`.

Donor `Models.cpp` is represented by project-owned session glue rather than
copied runtime code because its function-local statics bind the first
`Shared*` and retain adaptive state globally. The glue preserves the donor
construction and mix order but owns all 17 models, Mixer, SSE, and Shared
state per encoder or decoder block. HZ02 mode 36 / entropy 19 uses raw
transform 0, fixed `DEFAULT` block type, scalar factories, Q31-to-Q24 coder
quantization, and no optional LSTM. Block detection remains a separate future
decoder-visible contract.

## Decoder-visible block detection and specialist integration

Mode 37 / entropy 20 expands the accepted boundary to the complete 310-file
upstream `src` relative file set under `record_model` and the complete 25-file
`src/filter` directory under `block_detection/upstream/filter`. Across the
310-file snapshot, 302 files are byte-identical and exactly eight are the
documented scalar/session/precision adaptations in `DETECTED_SSE_AUDIT.md`.
The detector-only `FiltersDetection.hpp` is adapted from upstream
`src/filter/Filters.hpp`; both source identities are pinned by the donor
validator.

Project-owned `paq8px_block_detector.{h,cpp}` supplies a read-only in-memory
`File` and returns one validated specialist range. Project-owned
`paq8px_detected_sse_backend.{h,cpp}` stores that profile in the payload and
executes the donor Text, x86, Image, Audio, JPEG, or DEC model graph followed
by full SSE. Generic prefix/suffix bytes use the complete mode-36 path. The
decoder uses only stored profile metadata and never reruns detection. Exact
scope, excluded transform branches, framing, and hashes are in
`DETECTED_SSE_AUDIT.md`.
