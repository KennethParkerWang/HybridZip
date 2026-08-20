# PAQ8px MatchCore provenance

- Upstream: https://github.com/hxim/paq8px
- Revision: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- Revision date: `2026-06-25T21:18:53+02:00`
- Acquisition date: `2026-08-20` (Asia/Shanghai)
- Authoritative local checkout:
  `E:\MIXER\KU\hybridzip-r2\compressors\context-mixing\paq8px`
- License evidence: upstream `README.md`, `GPL-2.0-or-later`; preserved in
  `LICENSE.md`

The extraction was made from the following files at the fixed revision.
Hashes are SHA-256 of the complete upstream files, before adaptation.

| Upstream path | Role in extraction | SHA-256 |
| --- | --- | --- |
| `src/model/MatchModel.cpp` | 9/7/5 lookup order, candidate discovery, four-candidate cap, deduplication | `46ED3F8AF07B881231FAEDBDECC6605C7370D26B9310283AD18CC27DEAE60224` |
| `src/model/MatchModel.hpp` | constants and candidate/table layout | `1C675591DAF960BF86BC018F8449FFA614E851AB4B43D7D4996956E50EB80AE1` |
| `src/model/MatchInfo.cpp` | extension, mismatch, delta, recovery, saturation, and priority lifecycle | `4DF69305BA6B7AD7BC00EAC3F3BE773A7AF65C363678FAB9D477AFDE486B912E` |
| `src/model/MatchInfo.hpp` | match state fields and operations | `A63207569D7F2DAD30DB6DB6BC396ED09F3642E01143DA0D2BBDF186FE7C1817` |
| `src/HashElementForMatchPositions.hpp` | three recent positions per hash bucket | `281FC940E290F858CB421623D612F1263BB5E6B7448A534688F1B8EF21B35581` |
| `src/Hash.hpp` | `PHI64` and high-bit `finalize64` range reduction | `F48DEA5CC073EBB66835D527BC6281FCF769130B9D85C04B6CDF5CED8AAC20E2` |
| `src/model/NormalModel.cpp` | generic order-hash recurrence | `F3633CCC11FA3F46E45825D484BD63DAAD515573AC360E7A311176198B5872D5` |
| `src/model/NormalModel.hpp` | order-hash scope and update contract | `883B99E0303A3F1DA2F2EFD091DD66EB28E57FD42F39166DAD1EFE7C37FDE26D` |
| `src/BlockType.hpp` | generic `DEFAULT = 0` and `BlockType::Count = 30` constants | `9D759F154ADD53F0D18FAAD4D6B7D22B756F2C1E44BAA1EEE14ED219FE21F0BE` |
| `README.md` | copyright and GPL-2.0-or-later evidence | `898F9146C9B9DEB3AB419F3B7F3F61A8E1E690931E2D074169874502035D761A` |

`match_core.h` and `match_core.cpp` are modified extractions, not byte-identical
copies. `MODIFICATIONS.md` records the adaptation boundary. No PAQ8px
`Shared`, `Mixer`, context maps, stationary maps, indirect contexts, SSE/APM,
block detection, or special block models are included.

## HybridZip port evidence

The modified extraction is `match_core.h` and `match_core.cpp`; its behavioral
boundary is recorded in `MODIFICATIONS.md`. Project-owned integration is in
`src/r2/match/paq8px_match_service.{h,cpp}` and
`src/r2/experts/paq8px_match_expert.{h,cpp}`. Donor-golden lookup, candidate,
recovery, saturation, allocation, reset, cache, and adapter behavior is
covered by `tests/paq8px_match_tests.cpp`.

The project-owned `src/r2/entropy/donor_match_predictive_backend.{h,cpp}` also
consumes the PAQ8px Match evidence alongside the separately recorded cmix
Match posterior. Its encode/decode integration is covered by
`tests/donor_match_predictive_backend_tests.cpp`; this backend is not another
donor extraction.
