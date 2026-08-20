# libsais BWT provenance

This directory contains the smallest source closure used by HybridZip R2-B's
`BwtTransform` adapter:

| Imported path | Upstream SHA-256 |
| --- | --- |
| `LICENSE` | `3DDF9BE5C28FE27DAD143A5DC76EEA25222AD1DD68934A047064E56ED2FA40C5` |
| `include/libsais.h` | `88D57C7A43DB2877350529B3B39F4789B7FA6CADB4C94EFA5DBA322ADA7A6D86` |
| `src/libsais.c` | `1E5542ADE6F553A72C243F5EF896BF7AA8A3F7BF1D1F81899E61FAE6178B3867` |

- Upstream: <https://github.com/IlyaGrebnov/libsais>
- Revision: `b6e52ef33fe14f9d5c14c580d162b6fd2c27f2a8` (version 2.10.4)
- License: Apache License 2.0; the unmodified upstream text is `LICENSE`.
- Imported API: `libsais_bwt()` and `libsais_unbwt()` from `libsais.h`.
- Build boundary: `hz_libsais_donor` compiles the unmodified C source as a
  single-threaded static library. No OpenMP definition is enabled.

HybridZip-owned code is limited to
`src/r2/representation/bwt_transform.{h,cpp}`. It serializes the donor's
one-based primary index as four little-endian bytes in HZ02 block metadata;
the donor source files above are not modified.
