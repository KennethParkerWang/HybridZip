# CharLS JPEG-LS Donor Provenance

- Upstream project: CharLS
- Upstream URL: https://github.com/team-charls/charls
- Pinned revision: `c0bae6496fa5d787fbb4698debd1e5decb40cf3a`
- Source license: BSD-3-Clause
- License evidence: `LICENSE.md`
- License SHA-256: `C549F10B44BE965781388DD726DBD7FCECA5ABCD871E8477541E93A15E83A365`
- Import date: 2026-08-20

## Imported Closure

The exact `include/charls` and `src` trees are copied from the pinned donor,
along with `LICENSE.md`. The closure contains 69 files and 452,515 bytes.
HybridZip builds the donor as the `hz_charls_donor` static library.

| Path | SHA-256 |
| --- | --- |
| `src/charls_jpegls_encoder.cpp` | `D4740D3F6B781B0051270669AB1D26D75E1DBF7F168A16CA41A792B723D54656` |
| `src/charls_jpegls_decoder.cpp` | `2420E5E60F3C76BB41BC06E551153112C62520BA1BFE9E32BAB4C0FE0F01932E` |

## Integration Boundary

The imported source is byte-identical to the donor. The project-owned
`src/r2/representation/jpegls_transform.{h,cpp}` exposes it as HZ02 mode 17:
an 8-bit one-component raw frame with CRC32 plus uint32 little-endian width and
height metadata. The decoder checks the same geometry against the JPEG-LS SOF,
requires lossless one-component/no-interleave output, and bounds allocation by
the declared raw block length. The one permitted smoke is
`results/smoke/r2-jpegls-32k-20260820/verification.json`.
