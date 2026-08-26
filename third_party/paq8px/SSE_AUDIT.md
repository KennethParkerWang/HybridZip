# PAQ8px full SSE dependency audit

## Source identity

- Upstream: `https://github.com/hxim/paq8px`
- Revision: `29237fb44cb1995690e3eb72c6c3b1e4aede5791`
- License: `GPL-2.0-or-later`
- Authoritative checkout:
  `E:/MIXER/KU/hybridzip-r2/compressors/context-mixing/paq8px`
- Vendored destination: `third_party/paq8px/record_model`

## Accepted closure

The branch retains the complete donor `SSE` object rather than extracting an
individual APM. Its constructor and dispatch preserve the Text, color image,
palette image, grayscale image, Audio, JPEG, DEC, x86_64, and Generic APM,
APM1, and APMPost tables. The current raw/`DEFAULT` HZ02 profile executes the
Generic branch; the specialized tables remain compiled for later
decoder-visible block-type integration.

The eight upstream files total 18,032 bytes. Seven are byte-identical. Only
`APMPost.cpp` is adapted to remove the top-level `ArithmeticEncoder.hpp`
dependency and retain its archive-relevant `PRECISION = 31` constant locally.

| File | Role | Upstream SHA-256 | Vendored SHA-256 | Status |
| --- | --- | --- | --- | --- |
| `APM.hpp` | nonlinear context probability map declaration | `89BBA13D14352C376BDA60145BAAFE9FC23AA39D7A4E5BD2008C0A352D29F668` | `89BBA13D14352C376BDA60145BAAFE9FC23AA39D7A4E5BD2008C0A352D29F668` | byte-identical |
| `APM.cpp` | APM initialization, interpolation, and update | `5FBEA1B351715FC29E96E0F1054520344C2278E3FFC1D216475FFC93729F478D` | `5FBEA1B351715FC29E96E0F1054520344C2278E3FFC1D216475FFC93729F478D` | byte-identical |
| `APM1.hpp` | 33-point calibration table declaration | `0FD558B5FA78CC99DC6DD7FB76BC2C5E6584B764EEDFE4D92C42CDDD553B7C90` | `0FD558B5FA78CC99DC6DD7FB76BC2C5E6584B764EEDFE4D92C42CDDD553B7C90` | byte-identical |
| `APM1.cpp` | APM1 interpolation and update | `2EC95345E787B5F2B09666307A4FBC8C976C045D159B8C84440B3C5726A725ED` | `2EC95345E787B5F2B09666307A4FBC8C976C045D159B8C84440B3C5726A725ED` | byte-identical |
| `APMPost.hpp` | Q12-to-coder-precision post-map declaration | `88C2698644CC53511E6F2092D1268873DC22355810C1A305F248AA23E5C9C890` | `88C2698644CC53511E6F2092D1268873DC22355810C1A305F248AA23E5C9C890` | byte-identical |
| `APMPost.cpp` | count-ratio Q31 map and adaptive update | `8F8186C50A15597A9D8ABF97E825C613CAD6AEC2141E672AF89C36CC7BB19895B4` | `0E91D4129434BA0EEB0F291CAB3F412639B5C4A7FCB9B4D488F5A03C94EFC940` | adapted precision dependency |
| `SSE.hpp` | complete specialized and Generic table graph | `0329273463773D1E1483BF2DA6887905FEE4193E44CD9FB66F483797EE859E89` | `0329273463773D1E1483BF2DA6887905FEE4193E44CD9FB66F483797EE859E89` | byte-identical |
| `SSE.cpp` | complete constructor and block-type dispatch | `D76BEEAFED1826A18ABF97E825C613CAD6AEC2141E672AF89C36CC7BB19895B4` | `D76BEEAFED1826A18ABF97E825C613CAD6AEC2141E672AF89C36CC7BB19895B4` | byte-identical |

## HZ02 integration boundary

`src/r2/entropy/paq8px_similarity_sse_backend.{h,cpp}` constructs the complete
`SimilarityModelPair`, donor scalar Mixer, and complete SSE object for both
encoder and decoder. Each bit follows:

```text
SimilarityModelPair::mix -> Mixer_Scalar::p (Q12) -> SSE::p (Q31)
-> deterministic Q24 quantization -> HZ02 arithmetic coder
-> Shared::update using the same quantized probability shifted to Q31
```

The Q31 value is shifted right by seven and clamped to `[1, 2^24-1]` for the
existing HZ02 binary coder. Encoder and decoder use the same Q24 value;
`Shared::update` receives `p24 << 7`, and the donor miss bit is derived from
that same value. This prevents coder, SSE loss, and miss history from using
different probabilities.

The archive identifiers are mode 35, raw transform 0, and entropy 18. The CLI
policy is `--r2-mode=paq8px-similarity-sse`. The payload bound remains
`4 * uncompressed_size + 64` bytes. PAQ8px block detection is not part of this
profile, and historical D40 does not contain mode 35.

## Verification boundary

Release `hybridzip` and `hz_r2_codec_tests` compile and link with the complete
closure. The test executable is compile-only under the current minimal-test
rule. The single forced 1 KiB run produced a 652-byte archive with a 592-byte
payload and decoded byte-exactly. Its mode/transform/entropy tuple is
`35/0/18`; evidence is in
`results/smoke/r2-paq8px-similarity-sse-1k-20260821/verification.json`.
