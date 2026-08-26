# LZ4 Donor Provenance

- Upstream project: LZ4
- Upstream URL: https://github.com/lz4/lz4
- Release: `v1.10.0`
- Pinned revision: `ebb370ca83af193212df4dcbadcc5d87bc0de2f0`
- Selected source license: BSD-2-Clause (`lib/` only)
- License evidence: `lib/LICENSE`
- License SHA-256: `8B58C446121A109CCF32EDC094BBA3010A3D85E4EE3702950DB55E4D3E87736C`
- Import date: 2026-08-21

## Imported Closure

The five files below are byte-identical to the pinned donor. The repository's
non-library programs and tests use GPL-2.0-or-later and are not copied.

| Path | Bytes | SHA-256 |
| --- | ---: | --- |
| `LICENSE` | 1,311 | `8B58C446121A109CCF32EDC094BBA3010A3D85E4EE3702950DB55E4D3E87736C` |
| `lz4.c` | 118,145 | `9396F7DE527BC8435DE9C7569FB7998E56545A84B4F3C2D808C0235C01774539` |
| `lz4.h` | 46,014 | `26B82EFC53D1570F3B54EEF02E9C4764C1AD374FF03CAC04E2CED5EA4D4C552F` |
| `lz4hc.c` | 93,376 | `126CAFAFDB91767E6E55238298A910903851B35B2CEE27CE80AE2280469EE232` |
| `lz4hc.h` | 20,308 | `E43824E8A9BA16F54100C4CCBCCFA5782A858CA9AB83C48AAC303FEA3E76E21E` |

## Integration Boundary

`src/r2/entropy/lz4_backend.{h,cpp}` is project-owned HZ02 framing around
the donor's `LZ4_compress_HC()` level-12 parser and
`LZ4_decompress_safe()` decoder. The `HZ41` payload stores version, parser
level, exact raw and compressed sizes, and CRC32 for both byte sequences.
The outer HZ02 block retains its independent decoded-byte CRC32.

HZ02 mode 39 uses raw transform `0` and entropy ID `22`. The decoder rejects
unknown profiles, size mismatch, checksum mismatch, donor failure, or output
length mismatch. The payload bound is `24 + LZ4_compressBound(input_size)`.
