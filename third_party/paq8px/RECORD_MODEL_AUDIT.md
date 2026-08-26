# PAQ8px RecordModel dependency audit

## Source identity

- Upstream: `https://github.com/hxim/paq8px`
- Revision: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- Authoritative checkout:
  `E:/MIXER/KU/hybridzip-r2/compressors/context-mixing/paq8px`
- License: `GPL-2.0-or-later`; HybridZip uses the upstream "or later" option
  under its repository GPL-3.0 combined-work license.
- Audit date: `2026-08-21` (Asia/Shanghai)

## Completeness boundary

This branch ports the complete donor `RecordModel` prediction graph, not only
its record-length detector. The retained graph contains:

- 25 ContextMap contexts (`3 + 3 + 3 + 16`), each retaining all five donor
  mixer inputs;
- 6 StationaryMap instances;
- 4 SmallStationaryContextMap instances;
- 3 IndirectMap instances;
- 5 IndirectContext instances;
- the four position arrays and all record/padding transition state;
- 157 first-layer Mixer inputs, 1888 contexts, and 3 context sets;
- PAQ8px `StateTable`, `StateMap`, random replacement, hash buckets,
  scalar Mixer training, and `UpdateBroadcaster` update ordering.

The project adapter replaces only PAQ8px top-level concerns that are outside
the model:

- PAQ8px `File*`, command-line, block detector, and full archive framing are
  not linked;
- PAQ8px's top-level arithmetic coder is replaced by HybridZip's existing
  HZ02 binary arithmetic stream;
- `Shared::update` retains the donor bit/byte state transition and broadcaster
  order, while its unused top-level screen detection and archive-coder loss
  scaling are removed;
- `BlockType` is fixed to donor `DEFAULT`, automatic record-length detection
  remains enabled, and unavailable MatchModel state is represented by the
  donor invalid expected-byte value 256;
- the donor scalar Mixer is selected explicitly for deterministic portable
  C++17 behavior.

This boundary does not claim to port PAQ8px's full ContextModelGeneric,
MatchModel, NormalModel, SSE, LinearPredictionModel, SimilarityModel, block
detection, transforms, or archive format.

## Selected source closure

The selected closure contains 52 files and 117,508 bytes after adding the
`BitCount` dependency discovered by the first compile/link closure check.
All paths below are relative to upstream `src/`.

| Source group | Files | Status |
| --- | ---: | --- |
| Complete RecordModel | `model/RecordModel.hpp`, `model/RecordModel.cpp` | retained |
| Context models | `ContextMap`, `StationaryMap`, `SmallStationaryContextMap`, `IndirectMap`, `IndirectContext` headers/implementations | retained |
| Probability state | `AdaptiveMap`, `StateMap`, `StateTable`, `DivisionTable`, `Ilog`, `Stretch`, `Squash` | retained |
| Mixer | `Mixer`, `Mixer_Scalar` | retained |
| Shared lifecycle | `Shared`, `UpdateBroadcaster`, `IPredictor`, `RingBuffer` | retained with `Shared.cpp` runtime adaptation |
| Storage and hashing | `Array`, `Bucket16`, `Hash`, three `HashElement*` types, `Random` | retained |
| Constants/platform | `BlockType`, `CharacterNames`, `SIMDType`, `SystemDefines`, `Utils`, `ProgramChecker` | retained |

The authoritative pre-adaptation SHA-256 for every selected file is recorded
in `PROVENANCE.md` alongside the earlier MatchCore and APM1 sources. Direct
RecordModel hashes are:

| Upstream path | SHA-256 |
| --- | --- |
| `src/model/RecordModel.hpp` | `76A6A18242B31F2E9A3A78E499CDD0248891105A7EF70EC8E66E98A07C80E417` |
| `src/model/RecordModel.cpp` | `69E71C0BF154E19734A0ACDF62F9F0101E8AD5311169197AC1A0F214D4246826` |

## Decoder-synchronised lifecycle

For every bit, encoder and decoder perform the same sequence:

1. `RecordModel::mix()` subscribes all used maps and supplies exactly 157
   inputs plus the three donor contexts to `Mixer_Scalar`.
2. `Mixer_Scalar::p()` returns the donor 12-bit probability.
3. HybridZip's HZ02 binary arithmetic stream writes or reads one bit.
4. `Shared::update()` commits the bit, completes bytes in the ring buffer,
   updates character-group state, and broadcasts model/Mixer updates in
   subscription order.

The model is reconstructed from a fixed profile for every HZ02 block, so no
implicit process state is needed to decode an archive.
