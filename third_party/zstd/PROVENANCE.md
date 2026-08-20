# zstd Provenance

- Upstream: `https://github.com/facebook/zstd`
- Revision: `82d322c4973d9e2968d94047a40892bc6d9a9bdf`
- Local authority:
  `E:/MIXER/KU/hybridzip-r2/compressors/lz/zstd`
- Imported paths: `lib/`, `build/cmake/`, `LICENSE`, and `COPYING`
- Selected license: BSD-3-Clause (`LICENSE`)
- Import date: 2026-08-20

The donor implementation is built as a static single-threaded library. The R2
adapter enables frame content size and checksum and compares complete payload
bytes against other block candidates.

