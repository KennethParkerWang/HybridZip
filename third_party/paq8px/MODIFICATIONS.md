# PAQ8px MatchCore modification notice

Modified for HybridZip on 2026-08-20 from PAQ8px revision
`29237fb44cb1995690e3eb72c6c3b1e4aede5791`.

The adapted work remains available under the upstream GNU GPL version 2 or
later terms. License evidence and attribution are in `LICENSE.md`; exact donor
files and hashes are in `PROVENANCE.md`.

## Extracted behavior

- Replaced PAQ8px `Array`, `Shared`, and ring-buffer dependencies with C++17
  fixed-width types, `std::array`, and an exactly sized owned hash table.
- Preserved one shared table for the order-9, order-7, and order-5 hashes,
  queried in that order. Each bucket retains the three most recent positions.
- Preserved exact context verification, four active candidates, donor-index
  deduplication, registration strengths `9 -> 5`, `7 -> 3`, `5 -> 1`, and
  `MatchInfo::prio()` ordering.
- Preserved strength saturation at 65,535 and the recovery stability threshold
  of three matching bytes.
- Added a separate `contiguous_length`: it starts at the verified context
  length, increments on ordinary extension, becomes zero on mismatch, and
  restarts at one on the first recovered byte. PAQ strength continues from its
  backed-up value, as in the donor.
- Retained only PAQ8px's generic mode. The generic `NormalModel` recurrence is
  `(previous_order_hash + byte * 30 + order) * PHI64`, where 30 is the pinned
  donor's `BlockType::Count` and `DEFAULT` is zero.
- Guarded hash bits to 1 through 26. The upper limit matches PAQ8px's largest
  configured match table and prevents configurations beyond the donor's
  practical resource range. The default 20-bit table contains `2^20` buckets
  of 12 bytes, exactly 12,582,912 allocated bytes.
- Rejects positions beyond the 32-bit block-relative index used by the donor.

## Bit-to-byte recovery conversion

PAQ8px calls `MatchInfo::update()` before every predicted bit. On the first
wrong bit of a byte, it saves `length` and `index`, clears `length`, and sets
`delta` so the remaining bits of that same byte can use a delta context. At
the following `bpos == 0`, it clears `delta`; the candidate is then in
pre-recovery and predicts nothing for one complete byte. At the next byte
boundary, that skipped byte is compared with the donor byte one position past
the mismatch. Equality resumes recovery; inequality drops the candidate. A
second mismatch during recovery drops the candidate immediately.

HybridZip's service is called only before whole bytes, so there is no API
boundary inside the mismatching byte where delta predictions could be
returned. `MatchCore` performs the same delta transition internally while
atomically consuming that byte, and the next externally visible state is
pre-recovery. It then suppresses exactly one byte and applies the same donor
index/strength increments. Recovery remains visible until three contiguous
bytes have matched after the gap.

## HybridZip adapters

The project-owned adapters in `src/r2/match/paq8px_match_service.{h,cpp}` and
`src/r2/experts/paq8px_match_expert.{h,cpp}` add monotonic history traversal,
same-position caching, `IMatchService` field conversion, and an `IExpert`
predict-before-observe lifecycle. Match evidence is advisory: parse cost is
set to `UINT32_MAX`, so it is never treated as an LZ phrase.

`tests/paq8px_match_tests.cpp` covers the donor-golden lookup order, candidate
lifecycle, recovery, saturation, allocation guards, reset, cache behavior,
and both project-owned adapters.

`src/r2/entropy/donor_match_predictive_backend.{h,cpp}` consumes this Match
evidence alongside the separately governed cmix Match posterior. Its
project-owned probability fusion and encode/decode lifecycle are covered by
`tests/donor_match_predictive_backend_tests.cpp`.

## APM1 adaptation

`third_party/paq8px/apm1.{h,cpp}` is adapted from the pinned
`src/APM1.{hpp,cpp}`, `src/Squash.cpp`, and `src/Stretch.cpp`. The 33-point
table, donor float rounding, interpolation, and `rate=7` update are retained.
The donor `Shared` subscription is replaced by an explicit update call so
encoder and decoder advance the same state. The project-owned HZ02 backend
uses `(previous_byte << 8) | ((1 << prefix_length) | prefix_value)`, matching
the Generic SSE APM1 byte/prefix context shape, to calibrate the existing
Match-fused posterior. Full SSE and block detection remain outside the
accepted extraction.

## RecordModel adaptation

The `third_party/paq8px/record_model` files are retained donor sources from
the pinned revision, with `Shared.cpp` adapted only to remove the top-level
`ArithmeticEncoder.hpp` dependency and express its `PRECISION == 31` loss
scaling as local constants. `BitCount.cpp` is included because it is part of
the complete compile dependency closure. No model period detector or partial
RecordModel substitute is used.

`src/r2/entropy/paq8px_record_model_backend.cpp` owns the HZ02 lifecycle:
fixed `DEFAULT` block state, one MiB ring buffer, decoder-synchronised
predict/encode-or-decode/update ordering, donor scalar Mixer scale factors,
and Q12-to-Q24 probability conversion. HZ02 framing and CRC remain
project-owned. The retained boundary is detailed in
`third_party/paq8px/RECORD_MODEL_AUDIT.md`.

## LinearPredictionModel adaptation

The complete donor `LinearPredictionModel`, `ResidualMap`, scalar OLS, and
factory dependency closure is retained under `third_party/paq8px/record_model`.
Twelve of the 13 added files remain byte-identical. `OLS_factory.cpp` removes
compile-time SSE3 includes and runtime SIMD construction, retaining both
factory APIs but always returning donor scalar OLS implementations. This makes
the floating-point update order a fixed HZ02 mode 33 profile instead of a CPU
dispatcher choice.

`src/r2/entropy/paq8px_linear_prediction_backend.cpp` supplies the Generic
bias 256, scale factor 980, one fixed outer Mixer context, fixed `SIMD_NONE`,
one MiB ring buffer, Q12-to-Q24 conversion, and the HZ02 coder/CRC framing.
The three adaptive OLS predictors, four fixed predictors, seven residual maps,
14 Mixer inputs, and donor update ordering are otherwise retained.

## SimilarityModel adaptation

The complete SimilarityModel prediction graph is retained in the 10-file
closure recorded by `SIMILARITY_MODEL_AUDIT.md`: the slow/fast
`SimilarityModelPair`, 16+2 `ResidualMap` contexts, 8 `ContextMap2` contexts
per model, run statistics, byte history, donor mixer contexts, and scalar EMA
updates. The project-owned
`src/r2/entropy/paq8px_similarity_backend.cpp` replaces only PAQ8px's top-level
archive arithmetic coder and file/block framing with the decoder-synchronised
HZ02 bit loop. `Shared::chosenSimd` is fixed to `SIMD_NONE`, level 1 selects the
768-byte donor window, and the donor Q12 probability is converted to HZ02 Q24.

`SimilarityEmaFunctionsFactory.{hpp,cpp}` are the only adapted donor files:
SIMD includes and runtime selection are removed so scalar EMA updates are a
fixed archive profile. HZ02 mode 34 / entropy 17 is raw transform 0 with the
standard `4 * uncompressed_size + 64` payload bound and outer CRC32. Full SSE
and block detection remain outside this branch.

## Full SSE adaptation

The complete donor `APM`, `APM1`, `APMPost`, and `SSE` eight-file closure is
retained under `third_party/paq8px/record_model`. Seven files are
byte-identical. In `APMPost.cpp`, HybridZip removes only
`ArithmeticEncoder.hpp` and fixes the same donor precision value, 31, locally;
the count table, Q31 ratio, subscription, and update rules are unchanged.

`src/r2/entropy/paq8px_similarity_sse_backend.{h,cpp}` is project-owned glue.
It feeds the complete SimilarityModelPair/Mixer Q12 posterior into donor
`SSE::p`, quantizes Q31 to the existing HZ02 coder's Q24 scale, and updates
donor state from that exact quantized value. HZ02 mode 35 / entropy 18 uses
raw transform 0 and fixed `DEFAULT` block type. No block detector is claimed
by this branch.

## Generic context-model adaptation

The 77 files added for the complete non-LSTM Generic graph are byte-identical
to the pinned donor. HybridZip does not compile donor `Models.cpp`; its
function-local static objects would share `Shared*` and adaptive state across
codec sessions. Project-owned `paq8px_generic_sse_backend.{h,cpp}` creates the
same 17 models in donor order with one graph per encode/decode block, mixes
them in the exact `ContextModelGeneric.cpp` order, and then invokes full SSE.

Mode 36 / entropy 19 fixes level 1, a one MiB ring, scalar execution,
`BlockType::DEFAULT`, disabled optional LSTM, and the same Q31-to-Q24 update
boundary as mode 35. HZ02 framing and CRC are project-owned. No block detector
or specialist block-type claim is made here.

## Decoder-visible detector and specialist adaptation

`record_model` now preserves the complete pinned 310-file upstream `src`
relative file set. Exactly eight donor files differ: `APMPost.cpp`, `LMS.cpp`,
`MixerFactory.{hpp,cpp}`, `SimilarityEmaFunctionsFactory.{hpp,cpp}`,
`OLS_factory.cpp`, and `Shared.cpp`. Their changes only fix donor precision,
select scalar implementations, or remove top-level global coder coupling.
All other 302 files must remain byte-identical.

`block_detection/FiltersDetection.hpp` is a detector-only adaptation of
upstream `filter/Filters.hpp`. It redirects includes to the vendored source
snapshot, replaces `TransformOptions` with the two flags used by detection,
makes GIF/TIFF pending state local to one call, disables the CD and TAR
transform-only branches, disables recursive zlib probing, and excludes the
donor encode/decode/filter pipeline from compilation. The complete 25-file
upstream `filter` directory remains byte-identical under
`block_detection/upstream/filter` for source review.

Project-owned `paq8px_block_detector` and `paq8px_detected_sse_backend` provide
memory I/O, HZ02 substream framing, strict range checks, Q31/Q24 conversion,
and session ownership. Mode 37 stores the detector result in its payload so
decoder behavior cannot depend on re-detection or mutable process state.
