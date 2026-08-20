# HZ01 Archive Format

HZ01 is the first HybridZip single-file archive format. Multi-file archives,
random access, checksums, corruption recovery, and metadata preservation are
outside version 1.

## Byte Order

Every multi-byte integer in the header is unsigned little-endian. The header is
written field by field; it is never serialized from a native C++ structure.

## Header

| Offset | Size | Field | Version 1 value |
| ---: | ---: | --- | --- |
| 0 | 4 | magic | ASCII `HZ01` |
| 4 | 2 | version | `1` |
| 6 | 2 | header size | `40` |
| 8 | 8 | original size | decoded byte count |
| 16 | 4 | profile ID | `1` |
| 20 | 4 | flags | `0` |
| 24 | 8 | model seed | normally `0x485A5F56315F3031` |
| 32 | 1 | CDF bits | `24` |
| 33 | 1 | arithmetic state bits | `32` |
| 34 | 6 | reserved | all zero |

The arithmetic-coded payload starts at offset 40. A decoder rejects an unknown
magic, version, header size, profile, non-zero flag/reserved byte, CDF width, or
coder state width.

## Payload

For each original byte, the encoder and decoder independently construct the
same 257-entry cumulative frequency table. Every symbol has a positive
frequency and the final cumulative value is exactly `2^24`.

The table is passed to Project Nayuki's reference arithmetic coder configured
with a 32-bit state. Bits are emitted most-significant bit first within each
payload byte. The final arithmetic bit is followed by zero padding to the next
byte boundary.

## Model Synchronization

The archive does not contain model weights. Encoder and decoder reconstruct
state from the profile, model seed, and decoded prefix. For byte `t`, both sides
must execute:

```text
predict NGram, PPMD, Match, OnlineLSTM
normalize each 256-way distribution
mix and normalize
quantize deterministic CDF24
encode or decode byte t
update mixer with byte t
update all predictors with byte t
append byte t to ByteHistory
```

Changing this order changes the bitstream.
