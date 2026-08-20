# C-Blosc2 generic shuffle provenance

Extracted from C-Blosc2 commit `b17d0c3dae8d48800726a85455d9f1fdf0578b16`.

- Upstream: `https://github.com/Blosc/c-blosc2`
- Source: `blosc/shuffle-generic.c`
- Source SHA-256: `F29E4C258F36F86509111EE10AA76B892A0BDED07A498CAA2768400E266D86D9`
- License: BSD-3-Clause; upstream `LICENSE.txt` SHA-256 is
  `3BBCA6B627220480C39E9D50CF89376312FCE2465C0BD0278F0F0B711C4F3A2E`

The vendored generic loops have project-specific symbol prefixes. The owned
adapter is `src/r2/representation/blosc_shuffle_transform.cpp`.

The owned `src/r2/representation/blosc_bitshuffle_transform.cpp` adapts the
same donor's scalar element/bit transpose contract for 2/4/8-byte elements in
groups of eight. It stores the selected width in HZ02 metadata.
