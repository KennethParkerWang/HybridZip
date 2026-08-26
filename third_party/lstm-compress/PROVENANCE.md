# lstm-compress Donor Provenance

- Upstream project: `byronknoll/lstm-compress`
- Upstream URL: https://github.com/byronknoll/lstm-compress
- Pinned revision: `bbbbff0e9bc9a2052754068c1867e0e84344cabc`
- Source license: GPL-3.0
- License evidence: `COPYING`
- License SHA-256: `0B383D5A63DA644F628D99C33976EA6487ED89AAA59F0B3257992DEAC1171E6B`
- Import date: 2026-08-26

## Imported Closure

The raw coding closure is copied from the pinned checkout. The original
source paths and hashes are retained below; the port is project-owned C++17
code under `port/` and does not copy the donor application runner or optional
dictionary preprocessor.

| Donor path | SHA-256 |
| --- | --- |
| `src/predictor.cpp` | `66FA9070C47D4C85BCB5632BD7C154137C2193E9ED711FD14E0F6FE72EF07239` |
| `src/predictor.h` | `CC0F38E2115E431B9BD0C4C6908DB8AE35CC3FD58A5DB2082FC17017ABBF5DFC` |
| `src/coder/decoder.cpp` | `F7F0F3164C312AFEF710C44B63158E2D69F074BE4183B0E2852D3529488E3BAB` |
| `src/coder/decoder.h` | `105A410BBE0B336DC510A852B6815351A94EAF8176BFD8707D806DFCDF7FF246` |
| `src/coder/encoder.cpp` | `450DA3BB3D7DDE0FD91ABBFD5D531990094F8C1F455BFDCCE97B1892506CA1B9` |
| `src/coder/encoder.h` | `479DAB30236DCAC95A1BB0500FAF1328C68321DA6A9AD86ACF4E9936F2E8D91E` |
| `src/lstm/byte-model.cpp` | `F13714A8ECE2A5D3696035810F08743D8555C87610B361E27B317BDD1B3776A7` |
| `src/lstm/byte-model.h` | `FBF156DAE477421F42E1E4E7C3DF2842B0B7B077BA209D048419B5CAE9CA033A` |
| `src/lstm/lstm-layer.cpp` | `5E195F8E6F10339C887559FFC3BDBAABB06295C9D262AB4C743037A624F195A0` |
| `src/lstm/lstm-layer.h` | `17FF4600584C07BDBFA29019A77529673827D133BC1D61D5063A5405BCF503DB` |
| `src/lstm/lstm.cpp` | `432B971751B392E11BED776121CDF2EB3B7610AC75C3CEB6134187F5EC8C810D` |
| `src/lstm/lstm.h` | `699B0F0B14D1BF2B265AA557777514EF04AD1FDF7724361F2EEEA9ADF3B1FA86` |
| `src/lstm/sigmoid.cpp` | `8318333F7E1AECB1D705800E984C55A7A4FFE88B313FB4D38D1278735D0042F3` |
| `src/lstm/sigmoid.h` | `35DBB1AD2AB002379509AEFE6880B7767B25F65CD00925916A6DD87D578654FA` |

## Integration Boundary

`port/lstm_compress_donor_port.{h,cpp}` is a C++17 conversion of the donor's
raw no-preprocess codec. It preserves the four-gate 90-cell, three-layer LSTM,
horizon-10 online SGD update schedule, 256-way byte probability model,
`1 + 65534*p` discretization, and donor 32-bit bytewise range coder. The
adapter replaces file streams with `ByteView`/`std::vector<uint8_t>`, uses a
private deterministic RNG matching the MSVC `rand()` recurrence, and adds a
bounded four-byte EOF padding rule for strict archive decoding.

The HZ02 mode-23 backend wraps the port in `HLC1`: magic/version, vocabulary
flag, reserved bytes, optional 32-byte used-symbol bitmap, then the donor range
stream. The decoder validates the expected-size vocabulary rule and rejects
truncated or malformed payloads. Legacy untagged mode-23 payloads still use
the prior HybridZip decoder.

Evidence gate: one forced 1 KiB encode/decode at
`results/smoke/r2-lstm-compress-donor-port-1k-20260826/verification.json`.
It records `1024 -> 688 -> 1024`, mode/transform/entropy `23/15/1`, equal
input/decode SHA-256, archive SHA-256
`1ED24AEF7DD77A9DED7F0D0E8C85CB8631E0564E2034905A1C79C008AB72EBF0`, and
`byte_exact: true`.
