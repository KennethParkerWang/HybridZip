# HZ02 Archive Format

HZ02 is HybridZip's second-generation, block-oriented single-file format. It
supports stored, PROFILE_V1 predictive, donor-Match predictive, zstd, direct
FSE, and LZMA1 block payloads. A valid archive must decode to the original
bytes exactly.

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
block 0: 16-byte block header | 4-byte CRC32 metadata | payload
block 1: 16-byte block header | 4-byte CRC32 metadata | payload
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

The current profile requires `flags = 0x01` and `metadata_size = 4`. Therefore
the checksum is always present and appears immediately after the block header.
A decoder MUST reject a missing checksum flag, an unknown flag bit, or any
other metadata size.

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

Entropy ID `3` (rANS) is reserved by the implementation but is not a valid
HZ02 block backend yet. Unknown IDs and mismatched mode/entropy pairs MUST be
rejected. Transform parameters and IDs for future transforms must be carried
in block metadata; they cannot be encoder-only state.

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
payload bytes. The current six modes have equal 20-byte outer per-block
overhead, so comparing their payload sizes yields the same choice as comparing
complete block sizes. FSE framing and the 40-byte HZL1 envelope are already
inside their payload sizes. Future modes with different outer metadata must
compare total bytes.

Routing MUST NOT depend on benchmark filenames, paths, or test-case IDs. Any
dictionary ID, model family/version/hash, transform parameter, or other state
needed by the decoder must either be causally reproducible or encoded and
counted as archive bytes.
