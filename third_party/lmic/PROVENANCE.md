# LMIC donor provenance

- Project: Language Modeling Is Compression
- URL: https://github.com/google-deepmind/language_modeling_is_compression
- Revision: `b5c8f8a63349d0a2604367d47df4a7c79db52890`
- Download date: 2026-08-21
- License: Apache-2.0 (`E:/MIXER/KU/hybridzip-r2/neural/shared/lmic/LICENSE`)
- License SHA-256: `3DDF9BE5C28FE27DAD143A5DC76EEA25222AD1DD68934A047064E56ED2FA40C5`
- Donor source hashes:
  - `arithmetic_coder.py`: `67632A0629C6CE17EA0DA736DE896B508FAD4AED0C7FF3199B0C640BF908F792`
  - `utils.py`: `523E45934F29D051331A3145E83C40E4F9F2F5FA6CE380C9F18C77B184FB1DAA`
  - `compressors/language_model.py`: `3474BA7816368867AE2A4239919E661D6F96A7DD0DE7C368BA325619ED6A8672`
  - `constants.py`: `D56322E545222258C64E0A95ABF2B3D209B8A0E840B8743E776C464337FB5386`

The repository does not contain the pretrained `params.npz` required by its
Transformer compressor. HybridZip therefore ports the donor arithmetic coder
and probability-normalization boundary, but uses the existing decoder-
synchronised frozen bGPT bigram prior as an explicitly named posterior source.
The resulting branch is not a claim that the LMIC Transformer checkpoint was
reproduced.

- Smoke evidence: `results/smoke/r2-lmic-arithmetic-1k-20260826/verification.json`
  records the repaired mode-41 closure (`1024 -> 871 -> 1024`) and byte-exact
  input/decode hashes.
