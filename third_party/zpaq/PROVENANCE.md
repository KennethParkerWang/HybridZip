# ZPAQ/libzpaq provenance

- Upstream: <https://mattmahoney.net/dc/zpaq.html>
- Release: `zpaq 7.15` / `libzpaq` API 7.12
- Acquisition date: `2026-08-21`
- License: Unlicense/public domain for the main implementation; embedded
  divsufsort is MIT. Original notices remain in `libzpaq.cpp` and `COPYING`.

| File | SHA-256 |
| --- | --- |
| `libzpaq.cpp` | `151EB6BD83CB6C6F5261D64B1DB49358710F844EE1A2AA4B9CB63E17319DF122` |
| `libzpaq.h` | `08BD9CE17CE018468E35721E2C6A8BD13C0C5E397CE4E9C90C52AEC389662F79` |
| `COPYING` | `927B5FEDA84F7A7F2063998B124829182967F54B954DB2C3569E8BD07958BF07` |
| `readme.txt` | `F75E4A2BF50C4CF8A84861B7158E165191F52979D993AFA6A1F7D4F239081F9C` |

The accepted runtime closure is only `libzpaq.cpp` and `libzpaq.h`; the CLI
sources remain in the warehouse for reference. The project-owned adapter is
`src/r2/entropy/zpaq_backend.{h,cpp}`, with HZ02 mode 29 and an HZQ1 envelope.
