# FiniteStateEntropy donor

This directory vendors the `lib/` dependency closure from FiniteStateEntropy
revision `9f30e0918f87bd835fa040d922a208d7b219e50b`.

- Upstream: <https://github.com/Cyan4973/FiniteStateEntropy>
- Download date: 2026-08-20
- Selected license: BSD-2-Clause; see `LICENSE`
- Candidate modules: FSE and Huff0

The files under `lib/` and `LICENSE` are byte-identical to the fixed checkout
in `E:/MIXER/KU/hybridzip-r2/entropy/fse`. HybridZip does not edit those donor
sources. CMake applies an `HZFSE_`/`HZHIST_`/`HZHUF_` symbol prefix at compile
time because the separately vendored zstd library contains another internal
copy of the same public symbols. `hybridzip_fse_api.h` is the first-party
adapter declaration for that prefixed build.
