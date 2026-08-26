# PAQ8px SimilarityModel dependency audit

## Source identity

- Upstream: `https://github.com/hxim/paq8px`
- Revision: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- Authoritative checkout:
  `E:/MIXER/KU/hybridzip-r2/compressors/context-mixing/paq8px`
- License: `GPL-2.0-or-later`; HybridZip uses the upstream "or later" option
  under its repository GPL-3.0 combined-work license.
- Audit date: `2026-08-21` (Asia/Shanghai)

## Completeness boundary

This branch ports the complete donor `SimilarityModelPair` prediction graph,
not only the EMA period detector. The retained graph contains:

- one slow and one fast `SimilarityModel` with donor `SimilarityModelPair`
  ownership and update dispatch;
- 16 `ResidualMap` contexts plus 2 additional `ResidualMap` contexts per
  model;
- 8 `ContextMap2` contexts per model, including run statistics and byte-history
  inputs;
- donor match/record EMA buffers, best and second-best period state, run
  statistics, byte history, mixer contexts, scalar EMA updates, and the donor
  scalar `Mixer` substrate inherited from the accepted RecordModel closure;
- `SimilarityModel::MIXERINPUTS = 92`, `MIXERCONTEXTS = 102`, and
  `MIXERCONTEXTSETS = 2`; the pair-level mixer supplies one bias plus two model
  graphs (`1 + 2 * 92` inputs and `2 * 102` contexts).

The project adapter replaces only top-level concerns outside this prediction
graph:

- PAQ8px file I/O, command-line handling, full archive framing, and block
  detection are not linked;
- PAQ8px's top-level arithmetic coder is replaced by HybridZip's HZ02 binary
  arithmetic stream;
- SIMD runtime dispatch is removed from the EMA factory and the donor scalar
  EMA implementation is always selected for a fixed portable profile;
- donor `Shared` state is initialized with `DEFAULT`, `blockInfo = -1`, the
  first-byte block-position convention, and invalid `Match.expectedByte = 256`;
- the donor Q12 probability is converted to HZ02 Q24, while donor update order
  and decoder-synchronised state are retained.

This boundary does not claim to port PAQ8px's full ContextModelGeneric,
MatchModel, NormalModel, SSE, block detection, transforms, or archive format.
It also does not claim that the SimilarityModel is part of historical D40;
mode 34 was added after that ledger and has only the single smoke evidence
recorded below.

## Selected source closure

The SimilarityModel addition contains 10 donor files and 43,081 vendored
bytes. All paths below are relative to upstream `src/`.

| Source path | Role | Upstream SHA-256 | Vendored SHA-256 | Status |
| --- | --- | --- | --- | --- |
| `ContextMap2.hpp` | run-statistics and byte-history context map interface | `C2CC479E8F68E3BDCE87EA32D500585F6337F7782E6711E78756D247DBE8E28C` | `C2CC479E8F68E3BDCE87EA32D500585F6337F7782E6711E78756D247DBE8E28C` | byte-identical |
| `ContextMap2.cpp` | context-map implementation | `406F904B7314DD5D8DD6E16127516F98C928D39B87A2CF4A30665F44E2AAC623` | `406F904B7314DD5D8DD6E16127516F98C928D39B87A2CF4A30665F44E2AAC623` | byte-identical |
| `model/SimilarityModel.hpp` | 16+2 ResidualMap and 8 ContextMap2 graph declaration | `54F492E000107606D3C59EB908D0D6BC5DE79224F6912A02B30C04F616353C0C` | `54F492E000107606D3C59EB908D0D6BC5DE79224F6912A02B30C04F616353C0C` | byte-identical |
| `model/SimilarityModel.cpp` | complete per-model mix/update graph | `5FDF272FD3C0B9FE79333F6DE49CC156DAEF9B000B702AE183225B98A7839ABD` | `5FDF272FD3C0B9FE79333F6DE49CC156DAEF9B000B702AE183225B98A7839ABD` | byte-identical |
| `model/SimilarityModelPair.hpp` | slow/fast pair and level table declaration | `DF1B5F47E48D3B8C3FF1CFCF5A65DB854AA09ECFEA41AF38CC08A2E5E6752EEB` | `DF1B5F47E48D3B8C3FF1CFCF5A65DB854AA09ECFEA41AF38CC08A2E5E6752EEB` | byte-identical |
| `model/SimilarityModelPair.cpp` | pair lifecycle and EMA dispatch | `C4ACCA8154D5353790C9075BF78503C77A93B28617D39C725324E14DCC619D73` | `C4ACCA8154D5353790C9075BF78503C77A93B28617D39C725324E14DCC619D73` | byte-identical |
| `model/SimilarityEmaFunctions_Scalar.hpp` | scalar EMA API | `A180B4CC7C647D7110E29B34FA8AC1416B88D3A7C5F558CA51238B4F0DC52D35` | `A180B4CC7C647D7110E29B34FA8AC1416B88D3A7C5F558CA51238B4F0DC52D35` | byte-identical |
| `model/SimilarityEmaFunctions_Scalar.cpp` | scalar EMA update implementation | `675154811A4041B08927CCC06F0262A21EA97C9105337F21313E9473EB7A84AD` | `675154811A4041B08927CCC06F0262A21EA97C9105337F21313E9473EB7A84AD` | byte-identical |
| `model/SimilarityEmaFunctionsFactory.hpp` | EMA factory interface | `CAC493F507E2656A9220750AA9B058537D8C47E1C70FF0578C935CD1793814EF` | `9F17F82A0A50B721C62A14319D48F4B3F0C76EA49CBF43C8051A7340FFC438E2` | scalar-only adaptation |
| `model/SimilarityEmaFunctionsFactory.cpp` | EMA factory implementation | `62D91BC94A4FE54D405EF8DF480FAA8FAD8595199CE1623F17D66131015ABE16` | `1CE778890DD827FB725207980881FE6446D26D08239A6C9516A2EAD58B2223FE` | scalar-only adaptation |

The eight unchanged files retain their upstream bytes. The two adapted factory
files remove SIMD includes and runtime dispatch and always return the donor
scalar EMA implementation; no prediction formula or state transition was
rewritten. The common map/mixer/state files are reused from the already audited
RecordModel closure under the same pinned revision.

## HZ02 archive profile

The project-owned `src/r2/entropy/paq8px_similarity_backend.{h,cpp}` exposes:

- HZ02 mode `34`, raw transform `0`, entropy ID `17`, and CLI
  `--r2-mode=paq8px-similarity`;
- donor level `1`, maximum match distance `768`, one MiB ring buffer, scalar
  `Shared::chosenSimd = SIMD_NONE`, and donor Mixer scale `980, 90`;
- the decoder-synchronised loop
  `SimilarityModelPair::mix -> Mixer_Scalar::p -> HZ02 coder -> Shared::update`
  for every MSB-first bit;
- payload maximum `4 * uncompressed_size + 64`, exact declared output length,
  non-empty payload for non-empty blocks, and the mandatory outer CRC32.

The profile is fixed in the archive-visible mode ID; no CPU-dependent EMA
choice or encoder-only routing state is permitted.

## Verification gate

The branch gate is one forced 1 KiB encode/decode smoke after governance and
Release linking. Auto, D40, CTest, batch, and blocks larger than 1 KiB are not
part of this branch gate. The resulting archive path, hashes, timings, memory,
and byte-exact result belong in
`results/smoke/r2-paq8px-similarity-1k-20260821/verification.json`.
