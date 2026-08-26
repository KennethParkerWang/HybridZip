# fumin/ctw provenance

- Upstream: <https://github.com/fumin/ctw>
- Revision: `5fce9921d398dc3b720c188ebefd807dfc4f1f63`
- Acquisition date: `2026-08-21`
- License: BSD-3-Clause
- Donor language: Go

| Donor file | SHA-256 | Converted responsibility |
| --- | --- | --- |
| `ctw.go` | `F6AE89260F82BCE7B6A892D75C0751E8A955E54F816D0DEE2264B7FE9FF0FB68` | CTW node state, KT estimator, weighted probability, snapshot/revert, bit-context lifecycle |
| `ac/willems/willems.go` | `60A0033B8F5E1FB5B3B5E92757FF2AE114F4BD9782F8920FC6F168DDD31E13DD` | 12-bit exponential tables, delay/accumulator state, relabeling, termination, decoder synchronization |
| `LICENSE` | `B9A386F350DCA0BCEF67C3A0F903121D98429819D7C85386D6C520013736166E` | BSD-3-Clause redistribution terms |

The project-owned C++17 conversion is
`src/r2/entropy/ctw_backend.{h,cpp}`. It preserves the donor's least-significant
bit-first byte traversal, depth-48 zero bootstrap, CTW update/revert ordering,
KT probability update, probability relabeling, and Willems coder termination.

HybridZip adds a bounded `HZC1` envelope, little-endian sizes, encoded-bit
length, raw/stream CRC32 checks, strict HZ02 validation, and a 64 KiB block
limit. No Go runtime or donor application/data file is distributed.
