# Apache Arrow Parquet DELTA_BINARY_PACKED provenance

- Upstream: Apache Arrow
- URL: https://github.com/apache/arrow.git
- Pinned revision: `eafe3a9e620cf94683dee2347f370c35156dc965`
- License: Apache-2.0 (`LICENSE.txt`, SHA-256
  `23FC45DCE1769D9DDF4AAC4D6CDAF3F7F0D14FCC4D930DD0D4AFBEFA2EA3322A`)
- Import date: 2026-08-26

## Donor boundary

The warehouse retains the Parquet `DELTA_BINARY_PACKED` encoder/decoder
closure and its required headers under
`E:/MIXER/KU/hybridzip-r2/transforms/numeric/apache-arrow/raw`. The donor
defines block headers, zig-zag signed values, delta minima, four mini-blocks,
and little-endian bit packing for INT32 and INT64 values.

HybridZip does not copy Arrow source into its runtime. The accepted adapter is
the project-owned C++17 `DeltaBinaryPackedTransform` at
`src/r2/representation/delta_binary_packed_transform.{h,cpp}`, which preserves
the donor wire contract while adding strict bounds, exact bit-stream
consumption, and the HZ02 metadata boundary. The adapter is paired with the
existing Zstd FSE backend as HZ02 mode 42, transform 21, entropy 25.

The block metadata stores the original-byte CRC32, the selected element width
(4 or 8), and the transformed byte length. The decoder rejects invalid widths,
header mismatches, truncated bit streams, and trailing transformed bytes.
The one permitted correctness gate is recorded in
`results/smoke/r2-delta-binary-packed-zstd-1k-20260826/verification.json`.
