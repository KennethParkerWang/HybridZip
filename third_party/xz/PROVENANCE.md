# XZ Utils x86 BCJ provenance

Extracted from XZ Utils commit `11334a5d4d5ea3e8b2a3cbce74c1062d25cef772`.

- Upstream: `https://github.com/tukaani-project/xz`
- Source: `src/liblzma/simple/x86.c`
- Source SHA-256: `9E280398096F2EC958E45D86B940885055CB3E7A220EF597032E135E6E5DB4C6`
- License: 0BSD; upstream `COPYING.0BSD` SHA-256 is
  `54A423F7B2C890A1E4D6541F88F4E5702C0414988BA3F6247F4578D3B8BCA5B7`

`bcj_x86.c` keeps the donor's x86 conversion algorithm and replaces liblzma
framework setup with two project-specific one-shot entry points. The owned
adapter is `src/r2/representation/xz_x86_bcj_transform.cpp`.
