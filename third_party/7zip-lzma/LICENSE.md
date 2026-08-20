# 7-Zip LZMA subset license

The files in `C/` are an extracted subset of the 7-Zip C LZMA SDK. Every
copied source file states `Igor Pavlov : Public domain` (some headers include a
date in the same notice). Those per-file notices govern this subset.

The upstream repository-wide license text is preserved verbatim in
`LICENSE-7ZIP-SDK.txt`. It explains that files without an explicit per-file
notice default to LGPL and that some RAR files carry an additional unRAR
restriction. No default-LGPL, RAR, unRAR-restricted, BSD Zstandard, or BSD
XXH64 source is included in this subset.

The HybridZip wrapper in `src/r2/entropy/lzma_backend.*` is project code and
is covered by the HybridZip project license.
