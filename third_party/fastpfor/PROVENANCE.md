# FastPFOR provenance

This directory is the byte-identical Apache-2.0 source and header closure
copied from FastPFOR revision `2457e1ed1af35bbf7f4c509c863fa9797e637cb3`.

- Upstream: <https://github.com/fast-pack/FastPFOR.git>
- License: Apache License 2.0; `LICENSE` SHA-256:
  `DC1F5D2D43C5531DFE0ACAF4E950EA5DBE3E61E1850CF0E983BDA7EFC10D6693`
- Imported paths: `LICENSE`, `AUTHORS`, `README.md`, `headers/`, and `src/`.
- Build boundary: `hz_fastpfor_donor` compiles the scalar donor bit-packing
  unit. The project-owned adapter is
  `src/r2/entropy/fastpfor_backend.{h,cpp}`.

The adapter serializes donor uint32 words as little-endian HZ02 payload bytes,
stores an explicit raw tail for non-1,024-byte endings, and validates framing,
decoded count, and consumed donor words. No donor file in this directory is
modified.
