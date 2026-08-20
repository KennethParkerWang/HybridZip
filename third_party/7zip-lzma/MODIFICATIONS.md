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

No LGPL-default or unRAR-restricted 7-Zip source was copied or modified.
