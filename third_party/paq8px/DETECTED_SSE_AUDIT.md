# PAQ8px Decoder-Visible Block Detection and Specialist SSE Audit

## Decision

HybridZip accepts the pinned PAQ8px block detector and specialist prediction
graphs as HZ02 mode 37 / entropy 20. The encoder may detect one specialist
segment, but the decoder never repeats detection. The donor block type,
block information, segment start and length, and substream sizes are stored in
the payload and validated before allocation or decoding.

This branch executes the donor Text, x86, Image8/24/32, Audio8/16, JPEG, or
DEC Alpha model graph selected by the stored block type, followed by the
complete donor SSE. Generic bytes before and after the specialist segment use
the complete mode-36 Generic+SSE path.

## Source identity

- Upstream: `https://github.com/hxim/paq8px`
- Revision: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- Authoritative source root:
  `E:/MIXER/KU/hybridzip-r2/compressors/context-mixing/paq8px/src`
- License: `GPL-2.0-or-later`

`third_party/paq8px/record_model` contains the same 310 relative files as the
pinned upstream `src` tree, totaling 1,275,709 bytes. Of those, 302 are
byte-identical and eight are explicit portability/archive-profile
adaptations. The SHA-256 of the sorted UTF-8 `relative-path<TAB>vendored-hash`
manifest is:

`B8F3F11718710D5F9599BD16183C3E7DF17562F109FFEE20A4D3EE04571822EA`

The complete upstream `src/filter` directory is also retained byte-for-byte
under `third_party/paq8px/block_detection/upstream/filter`: 25 files and
207,710 bytes. `FiltersDetection.hpp` is the separately adapted detector-only
copy of upstream `src/filter/Filters.hpp`:

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| upstream `src/filter/Filters.hpp` | 107,930 | `B4CD6613F8B3DBDE2EC8FA224F007A0BC27ED9B1ABE8070B3130AF25622EAF1F` |
| vendored `block_detection/FiltersDetection.hpp` | 107,892 | `91D8F5A17E7B7603A08B76BAFD28AB3CB7116271BCFB94EE9E9D50BA440D9791` |

## Adapted donor files

The validator pins both upstream and vendored hashes for these eight files:

| Relative path | Adaptation |
| --- | --- |
| `APMPost.cpp` | Replace the removed top-level arithmetic encoder include with donor precision 31. |
| `LMS.cpp` | Remove runtime SIMD dispatch and construct the donor scalar LMS implementation. |
| `MixerFactory.cpp` | Remove runtime SIMD dispatch and construct the donor scalar Mixer implementation. |
| `MixerFactory.hpp` | Remove SIMD-only declarations from the fixed scalar profile. |
| `model/SimilarityEmaFunctionsFactory.cpp` | Select donor scalar EMA updates. |
| `model/SimilarityEmaFunctionsFactory.hpp` | Remove SIMD-only factory dependencies. |
| `OLS_factory.cpp` | Select donor scalar float/double OLS implementations. |
| `Shared.cpp` | Replace top-level coder coupling with decoder-synchronised HZ02 update state. |

All other files in the 310-file snapshot must remain byte-identical to the
pinned source. `tools/validate_r2_donors.ps1` enforces the exact relative file
set and every hash.

## Detector adaptation boundary

The adapted detector retains PAQ8px format signatures and range calculation,
but exposes only the detection half to an in-memory read-only `File` adapter.
It makes GIF/TIFF pending state local to one codec call. CD sectors, recursive
zlib decoding, TAR expansion, and the donor transform/temporary-file pipeline
remain present in the byte-identical upstream copy but are not compiled into
the mode-37 SSE profile because they are transforms rather than specialist
prediction graphs.

Supported stored specialist types are `TEXT`, `TEXT_EOL`, `IMAGE8`,
`IMAGE8GRAY`, `IMAGE24`, `IMAGE32`, `AUDIO`, `AUDIO_LE`, `JPEG`, `EXE`, and
`DEC_ALPHA`. Unsupported or invalid detections fall back to a full-block
Generic+SSE payload.

## HZ02 coding boundary

The mode-37 payload starts with a 22-byte little-endian profile header:

```text
u8 version = 1
u8 donor_block_type
u32 block_info
u32 specialist_start
u32 specialist_length
u32 generic_prefix_payload_size
u32 specialist_payload_size
generic prefix payload
specialist payload
generic suffix payload
```

The decoder validates the payload bound, stored type, segment range, substream
sizes, and empty-segment consistency. It reconstructs `Generic prefix |
specialist segment | Generic suffix` from stored metadata. No detector result,
filesystem state, CPU SIMD choice, or encoder-only state is required to
decode.

Probabilities follow the existing mode-35/36 boundary: donor Q12 mixer output
enters complete SSE, Q31 SSE output is shifted and clamped to HZ02 Q24, and
the same quantized value shifted back to Q31 is supplied to donor updates.
The archive-level CRC32 remains outside the backend payload.

## Runtime integration

- Detector adapter: `src/r2/entropy/paq8px_block_detector.{h,cpp}`
- Specialist backend: `src/r2/entropy/paq8px_detected_sse_backend.{h,cpp}`
- HZ02 mode / transform / entropy: `37 / 0 / 20`
- Forced CLI: `--r2-mode=paq8px-detected-sse`
- Auto family: match/context-model family, with archive-byte accounting
- Payload bound: `4 * uncompressed_size + 214` bytes

Historical D40 results predate mode 37 and must not be relabeled as evidence
for this branch.
