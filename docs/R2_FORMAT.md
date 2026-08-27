# HZ02 Archive Format

HZ02 is HybridZip's second-generation, block-oriented single-file format. It
supports stored, PROFILE_V1 predictive, donor-Match predictive, zstd, direct
FSE, LZMA1, LZ4 HC, and libsais BWT plus zstd block payloads, including Kanzi MTF and
RLT post-BWT stages. A valid archive must
decode to the original bytes exactly.

The words MUST, MUST NOT, SHOULD, and MAY in this document are normative.

## Separation From HZ01

HZ01 remains version 1 and is specified by [FORMAT.md](FORMAT.md) and
[PROFILE_V1.md](PROFILE_V1.md). HZ02 uses distinct `HZ02` magic and does not
change the HZ01 header, payload, or full-file model lifecycle.

An HZ02 `PredictiveV1` block reuses the PROFILE_V1 probability model, CDF, and
arithmetic coder, but it is not an embedded HZ01 archive. Its model is reset
from the HZ02 archive `model_seed` at the start of every block. Encoder and
decoder MUST perform the same per-symbol predict/observe sequence.

## Byte Order And Top-Level Layout

Every multi-byte integer is unsigned little-endian. Headers are serialized
field by field and never from native C++ structure memory.

```text
40-byte archive header
block 0: 16-byte block header | metadata | payload
block 1: 16-byte block header | metadata | payload
...
```

There are exactly `block_count` blocks and no index or trailer. Any byte after
the final declared payload is invalid trailing data.

## Archive Header

| Offset | Size | Field | HZ02 value or meaning |
| ---: | ---: | --- | --- |
| 0 | 4 | magic | ASCII `HZ02` |
| 4 | 2 | version | `2` |
| 6 | 2 | header size | `40` |
| 8 | 8 | original size | total reconstructed byte count |
| 16 | 4 | block size | nominal uncompressed bytes per block |
| 20 | 4 | block count | `ceil(original_size / block_size)` |
| 24 | 4 | flags | `0`; all archive flag bits are reserved |
| 28 | 4 | profile ID | `2` |
| 32 | 8 | model seed | seed used by deterministic predictive blocks; normally `0x485A5F56315F3031` |

`block_size` MUST be in `1..16 MiB`; the default is `64 KiB`. For an empty
input, `original_size` and `block_count` are both zero. For a non-empty input,
`block_count` MUST equal the ceiling expression above and fit in 32 bits.

## Block Header

| Offset | Size | Field | Meaning |
| ---: | ---: | --- | --- |
| 0 | 1 | mode | block coding path ID |
| 1 | 1 | transform | reversible representation ID |
| 2 | 1 | entropy | entropy backend ID |
| 3 | 1 | flags | bit 0 is `CRC32_PRESENT`; all other bits are reserved |
| 4 | 4 | uncompressed size | reconstructed bytes in this block |
| 8 | 4 | payload size | encoded payload bytes, excluding header and metadata |
| 12 | 4 | metadata size | metadata bytes between this header and payload |

The current profile requires `flags = 0x01`. Raw blocks set `metadata_size =
4`, containing only the checksum. BWT and BWT+MTF blocks set `metadata_size =
8`, containing the checksum followed by a four-byte primary index. BWT+RLT
blocks set `metadata_size = 12`, adding the four-byte transformed RLT length.
A decoder MUST
reject a missing checksum flag, an unknown flag bit, or a metadata size that
does not match the selected transform.

Each block's `uncompressed_size` MUST be the smaller of the archive block size
and the remaining original bytes. It is in `1..16 MiB`. `payload_size` MUST be
non-zero and must fit the selected backend's safe bound. Stored payload size
MUST equal uncompressed size.

## Mode, Transform, And Entropy IDs

Only these triples are valid in the current profile:

| Mode | Mode ID | Transform | Transform ID | Entropy | Entropy ID |
| --- | ---: | --- | ---: | --- | ---: |
| Stored | 0 | Raw | 0 | Stored | 0 |
| PredictiveV1 | 1 | Raw | 0 | SymbolArithmetic | 1 |
| Zstd | 2 | Raw | 0 | ZstdFse | 2 |
| Fse | 3 | Raw | 0 | Fse | 4 |
| Lzma | 4 | Raw | 0 | Lzma | 5 |
| DonorMatchPredictive | 5 | Raw | 0 | SymbolArithmetic | 1 |
| BwtZstd | 6 | Bwt | 1 | ZstdFse | 2 |
| BwtMtfZstd | 7 | BwtMtf | 2 | ZstdFse | 2 |
| BwtRltZstd | 8 | BwtRlt | 3 | ZstdFse | 2 |
| X86BcjZstd | 9 | X86Bcj | 4 | ZstdFse | 2 |
| ShuffleZstd | 10 | Shuffle | 5 | ZstdFse | 2 |
| BitshuffleZstd | 11 | Bitshuffle | 6 | ZstdFse | 2 |
| DeltaZstd | 12 | Delta | 7 | ZstdFse | 2 |
| FastPfor | 13 | FastPfor | 8 | FastPfor | 6 |
| Rans | 14 | Raw | 0 | Rans | 3 |
| Bcj2Zstd | 15 | Bcj2 | 9 | ZstdFse | 2 |
| RecordTransposeZstd | 16 | RecordTranspose | 10 | ZstdFse | 2 |
| JpegLs | 17 | JpegLs | 11 | JpegLs | 7 |
| FlacResidual | 18 | FlacResidual | 12 | FlacResidual | 8 |
| BrotliText | 19 | BrotliText | 13 | BrotliText | 9 |
| CmixWordDictionaryZstd | 20 | CmixWordDictionary | 14 | ZstdFse | 2 |
| NeuralLstm | 21 | Raw | 0 | SymbolArithmetic | 1 |
| SharedNeuralLstm | 22 | NeuralShared | 16 | SymbolArithmetic | 1 |
| LstmCompress | 23 | NeuralLstm | 15 | SymbolArithmetic | 1 |
| DeltaOfDeltaZstd | 24 | DeltaOfDelta | 17 | ZstdFse | 2 |
| BgptSharedPrior | 25 | NeuralSharedPrior | 18 | SymbolArithmetic | 1 |
| JaxCompressPortable | 26 | NeuralOnlinePortable | 19 | SymbolArithmetic | 1 |
| Ppmd7 | 27 | Raw | 0 | Ppmd7 | 10 |
| Ppmd8 | 28 | Raw | 0 | Ppmd8 | 11 |
| Zpaq | 29 | Raw | 0 | Zpaq | 12 |
| Ctw | 30 | Raw | 0 | Ctw | 13 |
| Paq8pxApmPredictive | 31 | Raw | 0 | Paq8pxApm | 14 |
| Paq8pxRecordModel | 32 | Raw | 0 | Paq8pxRecordModel | 15 |
| Paq8pxLinearPrediction | 33 | Raw | 0 | Paq8pxLinearPrediction | 16 |
| Paq8pxSimilarity | 34 | Raw | 0 | Paq8pxSimilarity | 17 |
| Paq8pxSimilaritySse | 35 | Raw | 0 | Paq8pxSimilaritySse | 18 |
| Paq8pxGenericSse | 36 | Raw | 0 | Paq8pxGenericSse | 19 |
| Paq8pxDetectedSse | 37 | Raw | 0 | Paq8pxDetectedSse | 20 |
| Wavpack | 38 | Raw | 0 | Wavpack | 21 |
| Lz4 | 39 | Raw | 0 | Lz4 | 22 |
| KanziAns | 40 | Raw | 0 | KanziAns | 23 |
| LmicArithmetic | 41 | Raw | 0 | LmicArithmetic | 24 |
| DeltaBinaryPackedZstd | 42 | DeltaBinaryPacked | 21 | ZstdFseDeltaBinaryPacked | 25 |
| FastExtension | 43 | Raw | 0 | ZstdFse | 2 |

Unknown IDs and mismatched mode/entropy pairs MUST be rejected. Transform
parameters and IDs for future transforms must be carried in block metadata;
they cannot be encoder-only state.

Mode 37 stores a decoder-visible 22-byte profile header inside its entropy
payload: version, PAQ8px block type, signed block information, specialist
start/length, Generic-prefix payload length, and specialist payload length.
The remaining bytes are the Generic prefix, specialist, and Generic suffix
substreams. The decoder MUST use these stored fields and MUST NOT rerun block
detection. Unsupported profiles, inconsistent empty segments, out-of-range
offsets, and payloads above `4 * uncompressed_size + 214` MUST be rejected.

### Stored

The payload is the original block verbatim.

### PredictiveV1

The payload is the PROFILE_V1 32-bit arithmetic-coded symbol stream without an
HZ01 header. The model starts from `model_seed` for each block. The decoder
produces exactly `uncompressed_size` symbols.

### DonorMatchPredictive

This path arithmetic-codes each byte MSB-first as eight binary symbols. At
each byte boundary it obtains the same PROFILE_V1 byte CDF as `PredictiveV1`,
PAQ8px multi-candidate `MatchEvidence`, and cmix's learned bit-level Match
posterior. The byte CDF and PAQ candidate votes are conditioned on the already
decoded bit prefix. The three Q24 posteriors are then combined with fixed
integer weights: `2:1:1` for V1:cmix:PAQ while PAQ evidence remains compatible
with the prefix, otherwise `3:1` for V1:cmix. The final bit frequencies are
clamped to `[2^20, 2^24 - 2^20]`; both symbols therefore remain encodable and
the payload has a decoder-enforced `4 * uncompressed_size + 64` byte bound.

cmix updates after every decoded bit. PROFILE_V1 and PAQ8px update only after
the complete byte is reconstructed. All three models reset at each HZ02 block
and derive their state causally from the archive model seed and decoded prefix;
the archive carries no hidden candidate bytes or encoder-only routing state.

### BwtZstd

The encoder applies the single-threaded Apache-2.0 `libsais_bwt()` donor to
the complete block, then writes the transformed bytes as one zstd frame. The
metadata is eight bytes: the mandatory original-byte CRC32 followed by the
donor's one-based BWT primary index as unsigned little-endian `uint32_t`.
The primary index MUST be in `1..uncompressed_size`. The decoder obtains
exactly `uncompressed_size` transformed bytes from zstd, calls
`libsais_unbwt()` with that primary index, checks the resulting size, and then
checks the CRC32 over final reconstructed bytes. The four primary-index bytes
are part of the archive cost and are included in automatic candidate selection.

### BwtMtfZstd

This mode applies Kanzi SBRT `MODE_MTF` to libsais BWT bytes before zstd. Its
metadata is the same `CRC32 + primary index` contract as BwtZstd. The decoder
must inverse MTF before inverse BWT.

### BwtRltZstd

This mode applies Kanzi `RLT` to libsais BWT bytes, then writes the shorter
RLT result as one zstd frame. The encoder MUST NOT select this candidate when
the donor reports that RLT cannot reduce the BWT bytes; a forced request then
fails rather than writing another mode. Metadata is twelve bytes: the mandatory
original-byte CRC32, the one-based BWT primary index, and an unsigned
little-endian RLT transformed length. The transformed length MUST be in
`1..uncompressed_size-1`. The decoder obtains exactly that RLT length from
zstd, inverse-RLTs it to `uncompressed_size` BWT bytes, inverse-BWTs it with
the primary index, and then validates the final CRC32. Both side-information
fields are included in automatic candidate cost.

### KanziAns

Mode 40 is a raw-byte Kanzi ANS range-coded block. Its HZK1 payload begins
with magic, version, order, log range, valid bit count, original size, raw
CRC32, stream CRC32, and reserved bytes, followed by the ANS bitstream. The
accepted profile is order 0, log range 12, and 16 KiB chunks. The complete
payload is bounded before decode and both CRCs plus exact bit consumption are
checked. It has one forced 1 KiB byte-exact smoke and no corpus-level
performance claim.

### X86BcjZstd

This mode applies the XZ Utils 0BSD x86 BCJ transform with start offset zero,
then stores one zstd frame. Its metadata is only the mandatory four-byte raw
CRC32. The decoder obtains exactly `uncompressed_size` BCJ bytes from zstd,
applies the decoder-direction BCJ transform, then validates the CRC32.

### Bcj2Zstd

This path invokes the public-domain 7-Zip BCJ2 donor, producing its `main`,
`call`, `jump`, and `range` streams. The streams are concatenated in that
order and compressed as one zstd frame. Metadata is twenty bytes: raw CRC32
followed by four uint32 little-endian uncompressed stream lengths. The decoder
requires the concatenated zstd output to match those lengths, requires call
and jump streams to be four-byte aligned, reconstructs exactly the declared
raw block size, consumes every donor substream, and then checks the CRC32.

### ShuffleZstd

This mode applies C-Blosc2 generic byte shuffle with a selected element width
of 2, 4, or 8 bytes before zstd. Metadata is five bytes: raw CRC32 followed by
the one-byte selected width. The decoder rejects every other width, decodes
exactly `uncompressed_size` bytes, unshuffles, and validates the final CRC32.

### BitshuffleZstd

This mode applies C-Blosc2's scalar bitshuffle contract to 2, 4, or 8-byte
elements in groups of eight, then zstd. Metadata is `CRC32 + width`; the
decoder rejects unsupported widths or a block shape that cannot satisfy the
same eight-element constraint before inverse bitshuffle and CRC validation.

### DeltaZstd

This mode applies the independent-block C-Blosc2 offset-zero XOR delta filter
to 1, 2, 4, or 8-byte elements before zstd. Metadata is `CRC32 + width`; the
decoder rejects all other widths, inverse-deltas in element order, and then
checks CRC32 over the reconstructed raw bytes.

### DeltaOfDeltaZstd

This numeric candidate is a second-order extension of the decoder-visible
delta representation. It interprets the block as little-endian unsigned
32-bit or 64-bit words, retains the first value and first delta, and encodes
each later delta minus its predecessor modulo the selected word width. The
width is one metadata byte after the mandatory CRC32; only widths 4 and 8 are
accepted. The reversible framing follows the local Parquet
`DELTA_BINARY_PACKED` donor's explicit typed-delta principle, while the
second-order view is project-owned so it can be compared as a complete HZ02
candidate. The transformed bytes are zstd-coded, all metadata is included in
candidate byte accounting, and the decoder reconstructs the exact original
word stream before CRC validation.

### DeltaBinaryPackedZstd

This numeric candidate converts the Apache Arrow/Parquet
`DELTA_BINARY_PACKED` contract into a project-owned C++17 representation for
little-endian INT32 and INT64 blocks. It stores the first value, the signed
minimum delta, four bit-packed mini-block widths, and their residuals. The
transformed stream is compressed by the existing Zstd FSE backend.

Metadata is nine bytes: the mandatory raw CRC32, one-byte element width
(`4` or `8`), and a four-byte little-endian transformed length. The decoder uses the stored width and length, validates
the Arrow-derived header and exact bit consumption, rejects trailing bytes,
inverse-transforms exactly the declared block size, and checks the final CRC32.
The payload safe bound is derived from the representation maximum plus the
selected Zstd bound; all metadata bytes are included in candidate accounting.

### FastPfor

This mode applies the Apache-2.0 FastPFOR `FastPFor<8>` donor to complete
1,024-byte groups interpreted as unsigned little-endian uint32 values. The
payload is the donor's resulting uint32 word stream, serialized little-endian.
Metadata is the raw CRC32 followed by a uint16 little-endian tail size and the
raw tail bytes. The tail is in `0..1023`; the remainder of the block must be a
multiple of 1,024 bytes. The decoder validates metadata shape, payload word
alignment, exact decoded uint32 count, and exact donor-word consumption before
appending the stored tail and checking the outer CRC32.

### Rans

This raw-block path uses the CC0-1.0 ryg-rans scalar byte implementation with
one static byte model per block. The payload starts with a one-byte scale-bits
value (`14`), a uint16 little-endian nonzero symbol count, then ascending
`symbol, uint16_le normalized_frequency` triples. Frequencies MUST be nonzero
and sum to `2^14`. The remaining payload begins with the donor's four-byte
little-endian state followed by its reverse-produced rANS byte stream. The
decoder rebuilds the cumulative lookup table from that payload model, emits
exactly `uncompressed_size` bytes, and MUST reject malformed frequency tables,
truncated renormalization input, or unconsumed rANS payload bytes.

### RecordTransposeZstd

This project-owned fixed-record transform follows the Apache Parquet
`BYTE_STREAM_SPLIT` contract for wider records. It accepts only 16- or 32-byte
records and writes byte column 0 for every record, then column 1, through the
last column, before storing one zstd frame. Metadata is `CRC32 + width`; width
MUST be 16 or 32 and the declared raw block size MUST be divisible by it. The
decoder obtains exactly `uncompressed_size` transposed bytes, restores row
order with the same width, and then validates the raw CRC32. It is distinct
from the C-Blosc2 2/4/8-byte generic shuffle modes.

### JpegLs

This path uses the BSD-3-Clause CharLS donor as a complete JPEG-LS coding
backend for one-component 8-bit raw image frames. The payload is one complete
JPEG-LS interchange stream, not a zstd-wrapped transform. Metadata is twelve
bytes: raw CRC32 followed by uint32 little-endian `width` and `height`.
Both dimensions MUST be in `1..100000` and their product MUST equal
`uncompressed_size`. The decoder requires the payload to end in JPEG EOI,
decodes it with CharLS subject to the raw-size bound, and rejects a JPEG-LS
SOF frame whose width, height, bit depth, component count, or interleave mode
does not exactly match the HZ02 contract. The encoder considers a finite set
of decoder-visible image widths and stores the winning geometry; automatic
routing requires byte-only low-text, low-entropy, spatial-smoothness gates,
then still competes complete payload plus metadata bytes.

### FlacResidual

This path uses the BSD-3-Clause libFLAC fixed/LPC residual donor closure for
raw little-endian signed 16-bit PCM, not a complete FLAC container. Metadata
is raw CRC32 followed by an 8-byte frame header (`version=1`, channel count,
independent or mid-side assignment, bits-per-sample `16`, and uint32 little-
endian frame count), then one channel record per plane. A record carries the
predictor kind, order, LPC quantization shift, Rice parameter, uint32 Rice-bit
count, signed warmup samples, and, for LPC, signed int32 coefficients. The
decoder rejects invalid shape, predictor order, Rice parameter, payload bounds,
nonzero bit padding, trailing bytes, or samples that cannot reconstruct the
declared 16-bit PCM block. Complete metadata and payload bytes participate in
candidate selection; automatic routing additionally requires low text fraction
and 16-bit adjacent-sample correlation.

### BrotliText

This path uses the MIT-licensed Brotli donor as a complete text-mode codec
with its static dictionary and literal-context machinery. The payload is one
self-contained Brotli stream generated with quality 11, default window bits,
and text mode. Metadata is only the mandatory raw CRC32: every setting needed
to decode is represented in the Brotli stream itself. The decoder allocates
exactly `uncompressed_size`, rejects a stream that needs more output or input,
requires success with no unconsumed payload bytes, and then validates the raw
CRC32. Encoder quality and the Auto preselection gate are not decoder state.
Automatic activation uses only block-local printable, whitespace, and
markup/code-symbol fractions plus entropy; the complete payload bytes still
compete against every enabled mode.

### CmixWordDictionaryZstd

This mode applies the GPLv3 cmix WRT word-dictionary donor with its fixed
`english.dic` resource, embedded at HybridZip build time, then stores one zstd
frame. Metadata is eight bytes: raw CRC32 followed by uint32 little-endian
transformed length. The transformed length MUST be in
`1..2*uncompressed_size`; the decoder obtains exactly that many bytes from
zstd, reconstructs exactly `uncompressed_size` raw bytes with the same
embedded dictionary, rejects residual donor input or buffered expanded output,
then validates CRC32. The dictionary identity is fixed by the product source
and its SHA-256 is recorded in `third_party/cmix/MODIFICATIONS.md`; it is not
an archive-external KU dependency. Automatic activation is limited to
printable, whitespace-rich text/code blocks and full archive bytes still
compete with the other candidates.

### Zstd

The payload is one self-contained zstd frame. The current encoder writes the
frame content size, enables the zstd frame checksum, and uses a single worker.
Compression level is not decoder state and is not serialized. The outer HZ02
CRC32 remains mandatory regardless of the zstd frame checksum.

### Fse

The payload begins with a one-byte framing kind:

| Kind | Name | Bytes after kind |
| ---: | --- | --- |
| 0 | Raw | exactly `uncompressed_size` original bytes |
| 1 | RLE | exactly one byte, repeated `uncompressed_size` times |
| 2 | Compressed | one FiniteStateEntropy block stream |

The decoder MUST reject an unknown kind, a wrong-sized Raw or RLE payload, an
FSE error, or an FSE result whose size differs from `uncompressed_size`. This
framing byte and any raw fallback bytes are part of `payload_size` and archive
cost.

### Lzma

The payload is a HybridZip `HZL1` envelope followed by one LZMA1 range stream.
The envelope is part of `payload_size`; it is separate from the outer HZ02
block metadata.

| Payload offset | Size | Field | Required value or meaning |
| ---: | ---: | --- | --- |
| 0 | 4 | magic | ASCII `HZL1` |
| 4 | 1 | version | `1` |
| 5 | 1 | flags | `0x07` |
| 6 | 1 | properties size | `5` |
| 7 | 1 | envelope size | `40` |
| 8 | 4 | uncompressed size | MUST equal the outer block value |
| 12 | 4 | compressed size | range-stream bytes after this envelope |
| 16 | 4 | uncompressed CRC32 | checksum of final block bytes |
| 20 | 4 | compressed CRC32 | checksum of the range-stream bytes |
| 24 | 5 | LZMA properties | donor-generated property byte and dictionary size |
| 29 | 11 | reserved | all zero |
| 40 | compressed size | range stream | LZMA1 stream with end marker |

In the flag byte, bit 0 (`0x01`) requires the LZMA end marker, bit 1 (`0x02`)
declares the uncompressed CRC32 at offset 16, and bit 2 (`0x04`) declares the
compressed-stream CRC32 at offset 20. Bits 3 through 7 are reserved. The
current decoder requires the exact flag byte `0x07`.

Both LZMA CRC fields use the CRC algorithm defined below. The payload MUST be
exactly `40 + compressed_size` bytes, the range stream MUST finish at its end
marker after consuming every declared byte, and decoded output MUST match both
the inner size/CRC and the outer HZ02 size/CRC. The five LZMA properties carry
all decoder parameters, including a dictionary size in `4 KiB..64 MiB`.
Compression level and encoder search settings are not decoder state and are
not serialized.

### Ppmd7

The payload is a HybridZip `HZP7` envelope followed by the byte stream emitted
by the public-domain 7-Zip PPMd7H model with its `Ppmd7z` range coder. The
current encoder profile is order 8 with 8 MiB model memory. Both values are
carried in the payload and are decoder state.

| Payload offset | Size | Field | Required value or meaning |
| ---: | ---: | --- | --- |
| 0 | 4 | magic | ASCII `HZP7` |
| 4 | 1 | version | `1` |
| 5 | 1 | flags | `0x01`, output-length termination |
| 6 | 1 | maximum order | `2..64`; current encoder writes `8` |
| 7 | 1 | envelope size | `32` |
| 8 | 4 | model memory | little-endian bytes, `2 KiB..64 MiB`; current encoder writes `8 MiB` |
| 12 | 4 | uncompressed size | MUST equal the outer block value |
| 16 | 4 | compressed size | range-stream bytes after this envelope |
| 20 | 4 | uncompressed CRC32 | checksum of final block bytes |
| 24 | 4 | compressed CRC32 | checksum of the range-stream bytes |
| 28 | 4 | reserved | all zero |
| 32 | compressed size | range stream | 7-Zip PPMd7z stream without an end marker |

The payload MUST be exactly `32 + compressed_size` bytes. Decoding stops only
after `uncompressed_size` symbols. An input callback underflow, early PPMd
end/error symbol, more than four unconsumed terminal range-flush bytes, either
CRC mismatch, unsupported order/memory, or a nonzero reserved byte invalidates
the block. The four-byte outer block CRC remains mandatory and independently
checks the reconstructed block before publication.

### Ppmd8

The payload is a HybridZip `HZP8` envelope followed by the byte stream emitted
by the public-domain 7-Zip PPMdI model and its native carryless range coder.
The current encoder profile is order 8, 8 MiB model memory, and CUT_OFF restore
method 1. All three values are carried in the payload and are decoder state.

| Payload offset | Size | Field | Required value or meaning |
| ---: | ---: | --- | --- |
| 0 | 4 | magic | ASCII `HZP8` |
| 4 | 1 | version | `1` |
| 5 | 1 | flags | `0x01`, donor end marker required |
| 6 | 1 | maximum order | `2..16`; current encoder writes `8` |
| 7 | 1 | restore method | `0` RESTART or `1` CUT_OFF; current encoder writes `1` |
| 8 | 4 | model memory | little-endian bytes, `1 MiB..64 MiB`; current encoder writes `8 MiB` |
| 12 | 4 | uncompressed size | MUST equal the outer block value |
| 16 | 4 | compressed size | range-stream bytes after this envelope |
| 20 | 4 | uncompressed CRC32 | checksum of final block bytes |
| 24 | 4 | compressed CRC32 | checksum of the range-stream bytes |
| 28 | 1 | envelope size | `32` |
| 29 | 3 | reserved | all zero |
| 32 | compressed size | range stream | PPMdI stream including its end marker |

The payload MUST be exactly `32 + compressed_size` bytes. A decoder must emit
exactly `uncompressed_size` byte symbols, then observe the donor end marker,
`Ppmd8_RangeDec_IsFinishedOK`, and exact input consumption. Callback underflow,
either CRC mismatch, unsupported order/memory/restore values, or a nonzero
reserved byte invalidates the block. The outer block CRC remains mandatory and
independently checks the reconstructed block before publication.

### Zpaq

The payload is a HybridZip `HZQ1` envelope followed by one complete ZPAQ level
2 block emitted by the pinned `libzpaq` 7.15 donor. The current encoder calls
`compressBlock()` with method level 3 and the decoder calls `decompress()`.

| Payload offset | Size | Field | Required value or meaning |
| ---: | ---: | --- | --- |
| 0 | 4 | magic | ASCII `HZQ1` |
| 4 | 1 | version | `1` |
| 5 | 1 | flags | `0` |
| 6 | 1 | method level | `0..5`; current encoder writes `3` |
| 7 | 1 | envelope size | `32` |
| 8 | 4 | uncompressed size | MUST equal the outer block value |
| 12 | 4 | compressed size | ZPAQ stream bytes after this envelope |
| 16 | 4 | uncompressed CRC32 | checksum of final block bytes |
| 20 | 4 | compressed CRC32 | checksum of the ZPAQ stream bytes |
| 24 | 8 | reserved | all zero |
| 32 | compressed size | ZPAQ stream | one donor-generated block and segment |

The payload MUST be exactly `32 + compressed_size` bytes. Unsupported methods,
size overflow, donor errors, output-limit violations, incomplete consumption,
either CRC mismatch, or a decoded size different from the outer declaration
invalidates the block. The mandatory outer CRC checks the reconstructed bytes
again before publication.

### Ctw

The payload is an `HZC1` envelope followed by the least-significant-bit-first
packed output of the converted fumin/ctw Willems coder. The model starts with
`depth` zero context bits and updates the Krichevsky-Trofimov counts and CTW
weighted probabilities after every bit. The current encoder uses depth 48 and
accepts at most one 64 KiB block. Auto skips this candidate for larger custom
block sizes; a forced CTW request over the bound fails before archive output.

| Payload offset | Size | Field | Required value or meaning |
| ---: | ---: | --- | --- |
| 0 | 4 | magic | ASCII `HZC1` |
| 4 | 1 | version | `1` |
| 5 | 1 | flags | `1`; encoded bit length is present |
| 6 | 1 | context depth | `1..64`; current encoder writes `48` |
| 7 | 1 | envelope size | `40` |
| 8 | 8 | uncompressed size | MUST equal the outer block value |
| 16 | 8 | encoded bit count | exact valid bits in the packed stream |
| 24 | 8 | packed stream size | bytes following the envelope |
| 32 | 4 | uncompressed CRC32 | checksum of final block bytes |
| 36 | 4 | packed stream CRC32 | checksum of all packed stream bytes |
| 40 | packed stream size | Willems stream | delay-register and accumulator termination included |

The payload MUST be exactly `40 + packed stream size` bytes. The bit count
MUST include at least the 77 termination/bootstrap bits and MUST NOT exceed the
packed byte capacity. Packed stream size MUST equal `ceil(bit count / 8)` and
unused high bits of the final LSB-first byte MUST be zero. Unsupported depth,
size mismatch, premature bitstream
exhaustion, either inner CRC mismatch, a reconstructed size mismatch, or the
mandatory outer CRC mismatch invalidates the block.

### PAQ8px APM1 predictive

Mode 31 uses the PAQ8px APM1 33-point probability map as a
decoder-synchronised calibration layer over the existing V1 + cmix/PAQ8px
Match posterior. It uses the raw transform (`0`) and entropy ID 14. The APM
context is `(previous_byte << 8) | ((1 << prefix_length) | prefix_value)`;
the table is initialized with the donor squash curve and updated after every
decoded bit. No side metadata is required beyond the mandatory block CRC32.

The payload is the shared HZ02 binary arithmetic stream. Its maximum accepted
size is `4 * uncompressed_size + 64` bytes. A decoder rejects an empty payload
for a non-empty block, enforces the bound, emits exactly `uncompressed_size`
bytes, and validates the outer CRC before publishing output.

### PAQ8px RecordModel

Mode 32 connects the complete PAQ8px `RecordModel` prediction graph from the
pinned donor, not only its record-length detector. The retained graph includes
25 ContextMap contexts, 6 StationaryMaps, 4 SmallStationaryContextMaps, 3
IndirectMaps, 5 IndirectContexts, the donor 157-input/1888-context/3-set
scalar Mixer, PAQ8px state tables, hash buckets, random replacement, and
update-broadcaster ordering. The dependency closure and direct source hashes
are recorded in `third_party/paq8px/RECORD_MODEL_AUDIT.md` and
`third_party/paq8px/PROVENANCE.md`.

The adapter fixes the donor block type to `DEFAULT`, initializes the relative
block position with the donor's first-byte convention, and sets
`Match.expectedByte = 256` (invalid/no-match) because the full PAQ8px Match
model is outside this closure. Each byte is coded MSB-first through the HZ02
binary arithmetic stream: RecordModel predicts, the stream encodes or decodes,
then the donor `Shared::update` broadcasts the bit update. The donor Q12
probability is converted to the HZ02 Q24 contract and no model state is carried
outside the decoder-visible block profile.

Mode 32 uses the raw transform (`0`) and entropy ID 15. Its payload maximum is
`4 * uncompressed_size + 64` bytes. A decoder rejects an empty payload for a
non-empty block, enforces this bound, emits exactly the declared byte count,
and validates the outer CRC before publishing output.

### PAQ8px LinearPredictionModel

Mode 33 connects the complete PAQ8px `LinearPredictionModel`: three adaptive
32-feature exponentially-forgetting scalar OLS predictors, four fixed linear
predictors, and seven ResidualMap contexts with 32 histograms each. The seven
maps supply 14 inputs to a fixed one-context scalar Mixer after the donor
Generic bias input 256; the Mixer uses the donor Generic scale factor 980.

At each byte boundary the model updates its OLS state from the previous byte,
loads feature strides 1, 2, and 3, calculates all seven byte predictions, and
selects each residual histogram from smoothed prediction error and block-position
parity. Every bit then follows `LinearPredictionModel::mix -> Mixer_Scalar::p ->
HZ02 binary arithmetic coder -> Shared::update`. The decoder reconstructs the
same OLS, histogram, and Mixer states solely from the decoded prefix.

`Shared::chosenSimd` is fixed to `SIMD_NONE`, and the vendored OLS factory is
adapted to construct scalar float/double implementations only. SIMD dispatch is
therefore not an implicit archive choice. The exact 13-file closure and hashes
are in `third_party/paq8px/LINEAR_PREDICTION_MODEL_AUDIT.md`.

Mode 33 uses raw transform (`0`) and entropy ID 16. Its payload maximum is
`4 * uncompressed_size + 64` bytes; non-empty blocks require a non-empty
payload, exact declared output length, and a valid outer CRC32.

### PAQ8px SimilarityModel

Mode 34 connects the complete PAQ8px `SimilarityModelPair` prediction graph,
not only its EMA period detector. Each block creates the donor slow and fast
models, retaining 16+2 `ResidualMap` contexts, 8 `ContextMap2` contexts with
run statistics and byte history per model, donor mixer contexts, and scalar EMA
updates. The pair-level graph supplies `1 + 2 * 92` inputs and `2 * 102`
contexts to the donor scalar Mixer.

The project profile fixes donor level 1 (768-byte maximum match distance), a
one MiB history ring, `Shared::chosenSimd = SIMD_NONE`, and donor scale factor
`980, 90`. Every bit follows `SimilarityModelPair::mix -> Mixer_Scalar::p ->
HZ02 binary arithmetic coder -> Shared::update`; the donor Q12 probability is
converted to HZ02 Q24. The scalar EMA factory adaptation removes SIMD runtime
dispatch, so the archive does not depend on host CPU features. The exact
10-file closure and hashes are in
`third_party/paq8px/SIMILARITY_MODEL_AUDIT.md`.

Mode 34 uses raw transform (`0`) and entropy ID 17. Its payload maximum is
`4 * uncompressed_size + 64` bytes; non-empty blocks require a non-empty
payload, exact declared output length, and a valid outer CRC32. Full PAQ8px SSE
and block detection remain outside this mode.

### PAQ8px SimilarityModel plus full SSE

Mode 35 keeps mode 34's complete SimilarityModelPair and donor scalar Mixer,
then passes the Q12 mixer posterior through the complete PAQ8px `SSE` object.
The vendored SSE retains all Text, image, audio, JPEG, DEC, x86_64, and Generic
APM/APM1/APMPost tables. This profile fixes `BlockType::DEFAULT`, so the
Generic branch determines the bitstream; specialized branches remain compiled
but cannot be selected without a future decoder-visible block-type contract.

`SSE::p()` returns Q31. Encoder and decoder both shift this value right by
seven and clamp it to `[1, 2^24-1]` for the HZ02 arithmetic coder. The donor
update receives that same Q24 probability shifted back to Q31, and the miss
bit is derived from the same value. No unquantized probability influences
decoder-synchronised loss or miss history.

Mode 35 uses raw transform (`0`) and entropy ID 18. Its payload maximum is
`4 * uncompressed_size + 64` bytes; non-empty blocks require a non-empty
payload, exact declared output length, and a valid outer CRC32. Exact donor
hashes and the single `APMPost.cpp` adaptation are in
`third_party/paq8px/SSE_AUDIT.md`.

### PAQ8px Generic context model plus full SSE

Mode 36 implements the donor's complete non-LSTM `ContextModelGeneric` path:
Normal, Match, SparseMatch, Sparse, SparseBit, Chart, Record, CharGroup, Text,
binary Word, Indirect, DmcForest, Nest, XML, LinearPrediction,
SimilarityModelPair, and final ExeModel. It preserves donor mixer sizing,
memory multipliers, scale `980, 90`, model ordering, and full SSE.

Every encoder or decoder block owns an independent graph. Donor `Models.cpp`
static locals are not used because they would bind the first `Shared*` and
leak adaptive state across later sessions. Optional LSTM is disabled.
`BlockType::DEFAULT` is fixed, so full SSE uses its Generic branch; no
encoder-only specialist detection is permitted.

Mode 36 uses raw transform (`0`) and entropy ID 19. Q12 Mixer output passes to
SSE Q31, is shifted and clamped to Q24 for the HZ02 coder, and the exact
quantized probability is shifted back for `Shared::update`. Its payload
maximum is `4 * uncompressed_size + 64`; non-empty blocks require a non-empty
payload, exact declared output length, and valid outer CRC32. The 77-file
expansion and session boundary are audited in
`third_party/paq8px/GENERIC_MODEL_AUDIT.md`.

### WavPack lossless PCM candidate

Mode 38 is the R2-C WavPack candidate. It uses raw transform `0` and entropy
ID `21`, and invokes the complete pinned WavPack lossless pack/unpack closure
through memory callbacks. The backend tries deterministic 8/16/24/32-bit
signed PCM profiles with one or two channels and chooses the smallest complete
candidate. This is a raw-byte candidate, not a standard `.wv` container.

The backend payload starts with a 16-byte `HZW1` header:

```text
bytes 0..3   magic "HZW1"
byte 4       payload version (1)
byte 5       bytes per sample (1..4)
byte 6       channel count (1..2)
byte 7       incomplete-frame tail size
bytes 8..11  little-endian WavPack stream size
bytes 12..15 reserved zero
bytes 16..   WavPack stream, followed by the raw tail bytes
```

The decoder rejects non-lossless streams, profile mismatches, invalid frame
counts, extra samples, trailing stream bytes, and output-size mismatches. The
candidate payload bound is `4 * uncompressed_size + 16 + 4096` bytes. The
donor closure and exact source hashes are recorded in
`third_party/wavpack/PROVENANCE.md`; the decoder-visible profile and tail are
counted inside the candidate payload.

### LZ4 HC block candidate

Mode 39 is the R2-A high-speed LZ coding path. It uses raw transform `0` and
entropy ID `22`, and invokes the pinned donor's `LZ4_compress_HC()` at level
12 plus `LZ4_decompress_safe()`. The profile is a raw LZ4 block inside HZ02,
not an `.lz4` frame.

The payload starts with a 24-byte `HZ41` header:

```text
bytes 0..3   magic "HZ41"
byte 4       payload version (1)
byte 5       LZ4 HC level (12)
byte 6       flags (0x03: raw and compressed CRC32 present)
byte 7       header size (24)
bytes 8..11  little-endian uncompressed size
bytes 12..15 little-endian compressed size
bytes 16..19 CRC32 of the original bytes
bytes 20..23 CRC32 of the LZ4 block
bytes 24..   LZ4 block
```

The decoder rejects unknown profiles, inconsistent sizes, either checksum
mismatch, donor failure, and output-length mismatch. The payload bound is
`24 + LZ4_compressBound(uncompressed_size)`. Exact donor file hashes and the
BSD-only copy boundary are recorded in `third_party/lz4/PROVENANCE.md`.

### FastExtension

Mode 43 is the append-only fast-path extension. Its outer block triple is
`(mode=43, transform=Raw, entropy=ZstdFse)`. The outer `Raw` transform is
intentional: the extension's reversible transform is self-described inside
Mode-43 metadata rather than consuming another HZ02 top-level transform ID.

Metadata begins with the mandatory four-byte raw CRC32, followed by this
descriptor:

```text
byte 0       extension version (1)
byte 1       codec ID (0 = standard zstd frame)
byte 2       extension transform ID
uLEB128      side-information byte count
byte[]       side information
```

The current transform IDs are `0 = none`, `1 = byte shuffle`, `2 =
bitshuffle`, `3 = XOR delta`, and `4 = x86 BCJ`. `none` and x86 BCJ have no
side information. Byte shuffle and bitshuffle require exactly one width byte
in `{2,4,8}`. XOR delta requires exactly one width byte in `{1,2,4,8}`. The
current descriptors are therefore four bytes for no-side-information forms
and five bytes for width-carrying forms; any malformed uLEB128 sequence,
unknown ID, unsupported width, or trailing descriptor byte is invalid.

The payload is one standard zstd frame. HZ02 owns the raw length and CRC32, so
the frame is written with its checksum, content-size, and dictionary-ID flags
disabled and with zero zstd workers. Every current extension transform
preserves byte count. The decoder first zstd-decodes exactly
`uncompressed_size` bytes, inverse-transforms according to the descriptor,
then verifies the outer raw CRC32 before publishing output. All descriptor and
payload bytes count toward Fast candidate selection.

## Block CRC32 Metadata

The four metadata bytes contain CRC-32/ISO-HDLC in little-endian order. The
calculation uses reflected polynomial `0xEDB88320`, initial value
`0xFFFFFFFF`, and final XOR `0xFFFFFFFF`.

The checksum covers the original uncompressed bytes of this block. With the
current Raw transform, these are exactly the bytes returned by the selected
backend. A future transformed block MUST verify the checksum after inverse
transformation, over the final reconstructed bytes.

A decoder MUST compare CRC32 before publishing any decoded output. A mismatch
invalidates the archive.

## Decoder Validation

A conforming decoder rejects at least:

- truncated headers, checksum metadata, or payloads;
- wrong magic, version, header size, profile ID, or reserved flags;
- an invalid block count or block-size contract;
- zero-sized blocks, zero-sized payloads, oversized payload declarations, or
  a stored payload whose size differs from the reconstructed size;
- unknown mode, transform, entropy, or backend-payload IDs and invalid ID
  combinations;
- missing or extra block metadata, checksum mismatch, and trailing bytes.

Failed decoding must not publish a partial destination file.

## Encoder Decisions And Accounting

An encoder MAY try several coding paths for a block because the selected mode
is explicitly recorded. The selection is not a decoder-invisible oracle.
Candidate comparison MUST include block headers, metadata, side data, and
payload bytes. Raw modes have equal 20-byte outer per-block overhead.

### Portfolio candidate ledger

The encoder keeps an in-memory ledger for each Auto-mode block. It records the
number of candidates that were actually activated and encoded, the selected
candidate size, the smallest size among those activated candidates, and their
byte gap. These values are runtime instrumentation only; they are not written
to the HZ02 archive and therefore do not change the decoder contract.

The CLI reports the aggregate fields as `candidates`, `selected`, `oracle`,
and `oracle_gap`. A forced single-candidate mode intentionally leaves the
complete Auto ledger unset. The `oracle` here is the minimum complete archive
candidate among candidates that were activated for that block, not the
decoder-invisible byte oracle described in the R2 research instrumentation.
BwtZstd and BwtMtfZstd have four additional primary-index bytes. BwtRltZstd
has eight additional bytes for the primary index and transformed length, so
candidate selection compares payload plus transform side information. FSE framing and the 40-byte
HZL1 envelope are already inside their payload sizes. Future modes with
different outer metadata must compare total bytes.

Routing MUST NOT depend on benchmark filenames, paths, or test-case IDs. Any
dictionary ID, model family/version/hash, transform parameter, or other state
needed by the decoder must either be causally reproducible or encoded and
counted as archive bytes.

## Neural LSTM Modes

`NeuralLstm` (mode 21) is a self-contained byte-arithmetic path. It uses the
fixed PROFILE_V1 Online LSTM configuration (200 cells, 2 layers, horizon 100,
learning rate 0.03) and the HZ02 archive `model_seed`; the decoder resets the
same model before each block.

`SharedNeuralLstm` (mode 22) is the shared-model contract. Its transform ID is
`NeuralShared`, and the four bytes after the block CRC contain the little-endian
model ID `0x31564C53` (`SLV1`). The decoder accepts only that ID and uses the
built-in runtime seed `0x4D4F44454C5631A5`; an unknown model ID is rejected
before decoding. The model identity bytes are included in `metadata_size` and
candidate byte accounting. Both modes use the existing deterministic CDF24 and
32-bit arithmetic coder; they are separate from the four-expert
`PredictiveV1` mode.

`LstmCompress` (mode 23) is the report-listed `lstm-compress` donor profile:
90 cells, 3 layers, horizon 10, learning rate 0.05, and gradient clip 2.0.
New payloads use the donor's 32-bit bytewise range coder and bit-level
probability path inside an `HLC1` envelope: magic (4 bytes), version (1),
vocabulary flag (1), reserved (2), an optional 32-byte used-symbol bitmap for
inputs at least 10,000 bytes, then the donor range stream. The donor retains a
256-slot byte interval; inactive bitmap symbols have zero probability after
the first observed byte. The decoder validates the envelope and expected-size
vocabulary rule. Older untagged mode-23 payloads continue through the legacy
HybridZip CDF decoder, preserving archive compatibility. The GPLv3 donor
closure is copied under `third_party/lstm-compress`; the archive decoder has
no external donor-file dependency and ignores `model_seed` for the new fixed
donor profile.

`BgptSharedPrior` (mode 25) is a separately identified shared-model-derived
coding path. It does not claim to run the complete 110M bGPT model. Instead,
`tools/extract_bgpt_shared_prior.py` loads the pinned MIT bGPT text checkpoint,
runs the donor's 12-layer patch decoder on a fixed 16-special-token bootstrap,
then runs its 3-layer byte decoder for the start state and all 256 previous-byte
states. The resulting 257 by 256 posteriors are deterministically quantized to
16-bit positive frequencies and embedded in the product. The table SHA-256 is
`1b135959d42b304d0bc4cce9da75022d4a8be7bc30061c5fe67405f1b2d06330`.

Block metadata after CRC is 36 bytes: little-endian model/projection ID
`0x31504742` (`BGP1`) followed by the complete 32-byte bGPT text checkpoint
SHA-256 `f30ed5a814086c5b9e64f56a76ccfcded00a82fc71c3ba6de322b708d29f6ac7`.
The decoder rejects any other ID or hash before arithmetic decoding. The first
byte uses context slot 256; later bytes use the preceding decoded byte as the
context. Each stored uint16 frequency is multiplied by 256 to form a CDF24
row, preserving the existing 32-bit arithmetic coder contract.

`JaxCompressPortable` (mode 26) is an explicitly reduced C++17 conversion of
the pinned jax-compress online-learning lifecycle. It uses a uniform first
symbol, an 8-symbol overlapping context, embedding width 8, two 16-unit LSTM
layers, prediction-before-observation Adam updates, and deterministic replay
of recent history every 4096 bytes. The complete canonical profile is recorded
in `third_party/jax-compress/PROVENANCE.md`; this mode does not claim numeric or
bitstream equivalence to the donor's batch-128, sequence-15, embedding-512,
8x1400 bfloat16 JAX/TPU profile.

Block metadata after CRC is 56 bytes: little-endian ID `0x3150434a` (`JCP1`),
the complete 20-byte source revision
`77adbc581eb0819a77e47c50ff6ed8ece338e60c`, and the complete 32-byte portable
profile SHA-256
`32f26c0071529f7cdf0b68b41518709ae8d09b050586b1a9896a7c5039f73be7`.
Including the mandatory CRC, `metadata_size` is 60 bytes. A decoder must reject
any other ID, source revision, or profile hash before arithmetic decoding.

`LmicArithmetic` (mode 41) is the donor-derived LMIC arithmetic-coder path
paired with the decoder-synchronised frozen bGPT byte prior. It is explicitly
identified as `lmic-arithmetic-frozen-bgpt-v1`; it is not a claim that the
unavailable LMIC Transformer checkpoint or complete JAX/Haiku runtime was
reproduced. The coder uses base 2 and precision 32 with the donor's inclusive
`[low, high]` interval, matching-digit normalization, carry-digit
normalization, and termination rules. The 257 context rows and 256 positive
frequencies are the same generated bGPT table used by `BgptSharedPrior`.

The entropy payload starts with an eight-byte `HLM1` envelope:

```text
bytes 0..3   magic "HLM1"
byte 4       framing version (1)
byte 5       number of left-padding bits in the final byte (0..7)
byte 6       arithmetic base (2)
byte 7       arithmetic precision (32)
bytes 8..    donor arithmetic bitstream
```

The decoder rejects an unknown envelope, invalid padding, a non-2 base, or a
non-32 precision. The payload bound is `16 * uncompressed_size + 64`; the
outer HZ02 CRC remains authoritative. No LMIC model files are required at
decode time.

## Automatic Representation Activation

Before `Auto` evaluates an expensive representation candidate, it computes
deterministic features from the block bytes: byte-histogram entropy,
printable-byte and zero-byte fractions, sampled repeated-window length and
coverage, x86 `E8`/`E9` branch-opcode density, and width-1/2/4/8 delta-byte
similarity. Repeated-window comparison uses a bounded 512-probe sample. The
feature computation has no archive state and uses neither file
names nor paths. It gates BWT family candidates to structured, low-entropy or
repetitive blocks; BCJ to sufficient branch density; shuffle, bitshuffle, and
delta to correlated or zero-rich blocks; rANS to byte distributions worth
modeling; and neural/shared-neural candidates to structured printable blocks.
Forced modes bypass this gate.

Activation is a workload-cost reduction only. Every activated candidate still
competes using complete real HZ02 block bytes, including its metadata. The
router is not decoder state and does not alter how a selected block decodes.
