# cmix Modification Notice

Modified for HybridZip on 2026-08-20 from cmix revision
`1d95fe95381a01442fceab585375cdec7c06922f`.

This notice is provided under GNU GPL version 3, section 5(a). The modified
work remains licensed under GNU GPL version 3; the full text is in `COPYING`.

Modified cmix-derived files:

- `ppmd_core.h` and `ppmd_core.cpp`: extracted the PPMD model from cmix's
  bit-level wrapper; added a standalone byte prediction/observation API,
  PROFILE_V1 memory/order configuration, allocation failure propagation, and
  fixed-width/layout checks; exposed the current maximum context depth as
  read-only instrumentation without changing model state.
- `lstm.h` and `lstm.cpp`: adapted construction and lifecycle for a seeded,
  byte-native predictor with zero auxiliary features; replaced process-global
  random initialization with a private deterministic stream.
- `lstm_layer.h` and `lstm_layer.cpp`: adapted the recurrent layer interface
  and state ownership required by the standalone predictor.
- `sigmoid.h` and `sigmoid.cpp`: retained the cmix sigmoid helper as part of
  the adapted Online LSTM dependency closure.
- `match_core.h` and `match_core.cpp`: extracted cmix Match into a standalone
  bit predictor with owned history, context-map, probability, and count state.

## Match source evidence

The Match extraction and its lifecycle were traced to these files at the
fixed revision. Hashes are SHA-256 of the complete upstream files before
adaptation.

| Upstream path | Role in extraction | SHA-256 |
| --- | --- | --- |
| `src/models/match.cpp` | Match prediction, observation, adaptive probability, and byte-update recurrence | `EBA1F125C83FF5082F5C5151ECEAEA92B51AA68162BA2778916F781849FAF7FC` |
| `src/models/match.h` | Match state and interface | `4057D583ED656D0E84D90364A257E2AD39ADCFDB62BAF32A7F1EA11C72121FD5` |
| `src/contexts/context-hash.cpp` | Rolling byte-context update | `2503EBF82E116C4CDD62F823877D3B8EF1993601C5B91C9D4570CF836223A7B1` |
| `src/contexts/context-hash.h` | Context-hash state and size contract | `13FA8C86982EF50E3C1B1ED1492D19F179ABF3A83740699019E1E15E7B01B8C7` |
| `src/predictor.cpp` | Match construction and Predict/Perceive update ordering | `8D6ACECFE3B8DF404CB0AD6CD4961C11634E7548954B56911CB256E41B77E674` |
| `src/predictor.h` | Predictor lifecycle interface | `E4F9734F9C70A23FD763BC99294BBDAF40FA0C35AE5722654EAE0A65D7110EB0` |
| `src/runner.cpp` | Encoder/decoder prediction and observation lifecycle | `AA30DF4944043AFD7CE888BBA0A6F6595AEEFAA4697F212EF8D1BF6EDB0B6BF4` |

## Match adaptation boundary

- Replaced manager-owned model wiring with standalone C++17 fixed-width state
  owned by `MatchCore`.
- Added configuration range checks and allocation-overflow guards for the
  history and 32-bit position map.
- Preserved bit/byte update ordering, context lookup, bucket initialization,
  and the exact float recurrence
  `p += (matched - p) * divisor` used by the donor.
- Added an explicit deterministic `reset()` that clears history, map, counts,
  and bit/byte lifecycle state and restores donor bucket initialization.
- Did not include cmix's full `Predictor`, mixer stack, or external `Shared`
  state.
- Added the project-owned wrapper
  `src/r2/experts/cmix_match_expert.{h,cpp}` for predict-before-observe
  enforcement, bounded Q24 conversion, and block reset.
- Added donor-golden, recurrence, allocation, reset, and lifecycle coverage in
  `tests/cmix_match_expert_tests.cpp`.

HybridZip-specific wrappers outside this directory are documented in
`../../docs/SOURCES.md`.
