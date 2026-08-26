# PAQ8px LinearPredictionModel dependency audit

## Source identity

- Upstream: `https://github.com/hxim/paq8px`
- Revision: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- Authoritative checkout:
  `E:/MIXER/KU/hybridzip-r2/compressors/context-mixing/paq8px`
- License: `GPL-2.0-or-later`; HybridZip uses the upstream "or later" option
  under its repository GPL-3.0 combined-work license.
- Audit date: `2026-08-21` (Asia/Shanghai)

## Complete prediction graph

This branch ports the complete donor `LinearPredictionModel`, not a substitute
delta predictor. Its decoder-synchronised graph contains:

- three exponentially-forgetting `OLS_float` predictors, each with 32 input
  features, solve interval 4, retention `1 - 1/162`, and regularization 0.001;
- feature strides 1, 2, and 3 for byte, 16-bit-high/gapped-byte, and RGB-like
  prediction;
- four fixed linear predictors: first-order, second-order, 16-bit-high, and
  RGB-like;
- seven ResidualMap contexts, each with 32 residual histograms and 256
  cumulative bins;
- two Mixer inputs per residual context, for exactly 14 model inputs.

The model itself declares zero Mixer contexts and zero context sets because it
is normally embedded in PAQ8px's larger Generic/JPEG Mixer. HybridZip supplies
one fixed `1 x 1` outer Mixer context plus the Generic bias input 256, retaining
the donor Generic scale factor 980.

## Selected source closure

The LinearPredictionModel addition contains 13 donor files. Twelve remain
byte-identical. `OLS_factory.cpp` is the only modified file: its exported float
and double factory APIs are retained but CPU dispatch and SSE3 constructors are
removed so HZ02 mode 33 always uses the scalar archive profile.

| Upstream path | SHA-256 |
| --- | --- |
| `src/Clz.hpp` | `63440E4D57B2D633630F087763442FFDDC9F73AE04E54B7C038271E73169499A` |
| `src/ResidualMap.hpp` | `925D13387DF7EFBD9ED444C1A51BD570F8E4C59A98B7BE85389606C51BC4F1FF` |
| `src/ResidualMap.cpp` | `F37923B587E2D6E630BB011044C2968846DD8D292AD42807F5ACCB6299D8EF07` |
| `src/OLS.hpp` | `F4A99ADC9A01AB1392D46B5CD0EE3ABD896665A77FC2054C3FF8D0F1089A06D0` |
| `src/OLS.cpp` | `D4904221D1C344890A120CA6759ED37D71A2CF30EDED5389F613DE1D85F55553` |
| `src/OLS_factory.hpp` | `B9CB3FFD2BCA1DBCAC3D10A932DAA81416AD4B6E1352AE5D3482DE9DB63B9D83` |
| `src/OLS_factory.cpp` | `9C622D3EB47CAD7478B5F9D8A6740C0FD8EC027C57ABEE001F68856C17CFB492` |
| `src/OLS_float_Scalar.hpp` | `CC08FDBD498D82F573D45AE05D01CC4D7339CDC2D1B6BCBE5A42C190321482EE` |
| `src/OLS_float_Scalar.cpp` | `D8A1709A33D6F703EE464F82341990E435B9677C4232F8F0F44FF2CEBC21D316` |
| `src/OLS_double_Scalar.hpp` | `6B5623C6BEC095E939EB212E2FD527C0D3D456BCA205DD28943F6CF131E11591` |
| `src/OLS_double_Scalar.cpp` | `9416EC6581406CB531E45210B88B6994946C7B5C8490B43C25C6B04C75478CF8` |
| `src/model/LinearPredictionModel.hpp` | `7D23FE1B17378F343770A0ED53667EE73A5D00277A17E5C9559269825203653F` |
| `src/model/LinearPredictionModel.cpp` | `E4EB8F6522C6775B91E63DA074AB5FB628826DEBCE048362BB2BF74FF5705F36` |

`OLS_factory.cpp` has vendored SHA-256
`9D7577FCBE76F4C50F51B161ECF873E73D19AF16DCED2CE76B0CA6757D0CFC38`.
The other twelve vendored hashes equal their upstream hashes.

## Archive semantics

For each bit, encoder and decoder perform the same sequence:

1. On a byte boundary, update OLS coefficients from the previous byte, load
   the three 32-feature vectors, generate seven predictions, and select seven
   error/parity histograms.
2. `ResidualMap::mix()` supplies exactly 14 inputs after the fixed bias.
3. The scalar Mixer produces a Q12 probability; HZ02 converts it to Q24.
4. The HZ02 binary arithmetic stream writes or reads one bit.
5. `Shared::update()` commits the bit and broadcasts byte-complete histogram
   and Mixer updates in donor order.

`Shared::chosenSimd` is explicitly fixed to `SIMD_NONE`. The build does not use
unsafe/fast floating-point flags. The model, ResidualMap state, OLS state, and
Mixer state reset for every block; no encoder-only state is needed to decode.

HZ02 mode 33 uses raw transform 0, entropy 16, mandatory outer CRC32, and the
decoder-enforced payload bound `4 * uncompressed_size + 64`.
