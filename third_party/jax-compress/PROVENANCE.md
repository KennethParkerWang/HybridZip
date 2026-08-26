# jax-compress Portable Profile

- Source: `https://github.com/byronknoll/jax-compress`
- Revision: `77adbc581eb0819a77e47c50ff6ed8ece338e60c`
- Source archive SHA-256:
  `32C12D882FBB9BF67D2F8465D8CA7777EC8918CC7C62CE43D75363500DBDA1A9`
- License: Unlicense; retained in `LICENSE`.
- Download date: 2026-08-21.
- Warehouse source:
  `E:/MIXER/KU/hybridzip-r2/neural/online/jax-compress`

The C++17 backend in
`src/r2/entropy/jax_compress_portable_backend.{h,cpp}` converts the causal
lifecycle in notebook cells 3, 9, 10, 11, 14, and 17: a uniform first symbol,
overlapping sequence windows, stacked LSTM prediction, integer arithmetic
coding, prediction-before-observation, Adam test-time updates, and periodic
history replay on both encoder and decoder.

The portable profile is intentionally smaller than the donor's default TPU
profile. Its canonical identity string is:

```text
jax-compress-portable-v1;batch=1;seq=8;embedding=8;units=16;layers=2;adam_b1=0;adam_b2=0.9999;eps=1e-12;lr=0.0005;clip=4;retrain_period=4096;retrain_block=256;retrain_stride=8;numeric=params-f32-accum-f64;seed=1234
```

The identity string SHA-256 is
`32F26C0071529F7CDF0B68B41518709AE8D09B050586B1A9896A7C5039F73BE7`.
HZ02 mode 26 stores `JCP1`, the complete 20-byte source revision, and the
complete 32-byte profile hash after the block CRC. The implementation is
compiled from `src/r2/entropy/jax_compress_portable_backend.{h,cpp}`. This port
does not claim numeric or bitstream
equivalence to the donor's default batch-128, sequence-15, embedding-512,
8x1400 bfloat16 JAX/TPU profile. The periodic replay adaptation uses the same
portable Adam state and no dropout; that divergence is part of the versioned
profile above.

The profile fixes float parameter storage and double accumulation, but the
current archive compatibility claim is limited to the compiled HybridZip
runtime. Cross-libm and cross-compiler CDF identity has not yet been proven.
