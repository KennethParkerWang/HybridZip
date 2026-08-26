# Modifications and integration

The files under `C/` are unmodified donor files. HybridZip compiles them as C
with `Z7_ST` defined. This selects the donor's single-threaded implementation,
so `LzFindMt`, `Threads`, and their dependencies are not required.

HybridZip adds a separate C++17 adapter in
`src/r2/entropy/lzma_backend.cpp`. The adapter:

- calls the donor `LzmaEncode` and `LzmaDecode` one-call APIs;
- forces one encoder thread and writes an LZMA end marker;
- stores the five donor-generated LZMA properties;
- bounds input/output and dictionary sizes before allocation;
- starts from `input + ceil(input / 3) + 128` bytes and, when the donor
  reports `SZ_ERROR_OUTPUT_EOF`, retries with geometrically larger buffers up
  to the adapter safety limit (`2 * input + 1 MiB`, plus the payload header);
- wraps the range stream in a versioned, fixed-size header with exact lengths;
- checks both compressed bytes and decoded bytes with CRC-32;
- rejects truncation, trailing bytes, malformed properties, size mismatches,
  streams without an end marker, and nonzero reserved fields.

HybridZip also adds the separate project-owned C++17 adapter
`src/r2/lz/lzma_match_finder_service.cpp`. It uses the same unmodified
single-threaded `LzFind` donor API with a fixed binary-tree configuration,
collects each donor match list, and applies a deterministic longest-match,
lowest-distance greedy policy. It exposes a reconstructable parse candidate
only; it neither changes the LZMA encoder nor claims to reproduce its optimal
parser.

No LGPL-default or unRAR-restricted 7-Zip source was copied or modified.

HybridZip also compiles the unmodified public-domain BCJ2 encoder/decoder
closure (`Bcj2.c`, `Bcj2.h`, and `Bcj2Enc.c`). The project-owned
`src/r2/representation/bcj2_transform.cpp` frames the donor's main, call,
jump, and range substreams. HZ02 metadata retains every substream length;
the coding path is zstd-backed and requires exact four-stream consumption.

HybridZip additionally compiles the unmodified public-domain PPMd7H closure
(`Ppmd.h`, `Ppmd7.h`, `Ppmd7.c`, `Ppmd7Enc.c`, and `Ppmd7Dec.c`). The
project-owned `src/r2/entropy/ppmd7_backend.cpp` adapter:

- calls the donor `Ppmd7_*` model and `Ppmd7z_*` 7z range-coder APIs;
- uses decoder-visible order 8 and 8 MiB model memory by default;
- wraps the length-terminated stream in a fixed `HZP7` envelope containing
  version, order, memory, exact raw/stream lengths, and CRC32 for both;
- bounds model memory, decoded size, range-stream expansion, and callback
  reads/writes before allocating or publishing output;
- rejects truncated input, early PPMd termination, excess trailing range
  bytes, checksum mismatches, unsupported profiles, and nonzero reserved data.

HybridZip also compiles the unmodified public-domain PPMdI closure
(`Ppmd8.h`, `Ppmd8.c`, `Ppmd8Enc.c`, and `Ppmd8Dec.c`). The project-owned
`src/r2/entropy/ppmd8_backend.cpp` adapter:

- calls the donor `Ppmd8_*` model and native carryless range-coder APIs;
- uses decoder-visible order 8, 8 MiB memory, and CUT_OFF restore method 1;
- emits and requires the donor end marker, finished range state, and exact
  compressed-stream consumption;
- wraps the stream in a fixed `HZP8` envelope containing version, flags,
  order, restore method, memory, exact raw/stream lengths, and CRC32 for both;
- bounds model memory, decoded size, range-stream expansion, and callback
  reads/writes before allocating or publishing output.

The donor files remain byte-identical. HybridZip does not compile
`Ppmd7aDec.c`, RAR, or any 7-Zip C++ wrapper for either PPMd mode.
