# Sources And Adaptations

## Local Material Policy

Upstream material is stored under `E:\MIXER\KU` before use. Source URL,
revision, license, and hashes are recorded beside each authoritative checkout.
Donor directories are not edited in place; adapted copies live in
`third_party/`.

## zstd v1.5.7 Staging Donor

- Local artifact: `E:\MIXER\KU\zstd-v1.5.7\zstd-1.5.7.tar.gz`
- Upstream: `https://github.com/facebook/zstd`, tag `v1.5.7`, peeled commit
  `f8745da6ff1ad1e7bab384bd1f9d742439278e99`
- Archive: 2,434,947 bytes, SHA-256
  `EB33E51F49A15E023950CD7825CA74A4A2B43DB8354825AC24FC1B7EE09E6FA3`
- License: BSD-3-Clause, `LICENSE` SHA-256
  `7055266497633C9025B777C78EB7235AF13922117480ED5C674677ADC381C9D8`
- Repository provenance: `docs/provenance/zstd-v1.5.7.json`

This is a verified warehouse artifact, not a production import. HybridZip
continues to build the separately recorded `third_party/zstd` v1.6.0 source
until a distinct source-replacement patch records its copied files, build
changes, compatibility gates, and new archive evidence.

## Project Nayuki Reference Arithmetic Coding

- Local upstream: `E:\MIXER\KU\nayuki-ac`
- Revision: `ab6ee50afec04d235a4b82d17f407f0fd2b42e9a`
- License: MIT
- Provenance: `E:\MIXER\KU\nayuki-ac\SOURCE_PROVENANCE.md`
- Copied files: `ArithmeticCoder.*`, `BitIoStream.*`, `FrequencyTable.*`

HybridZip adds an immutable `CdfFrequencyTable` view over its CDF24 and thin
stream wrappers. The upstream demo applications and PPM model are not copied.

## cmix

- Authoritative local upstream: `E:\MIXER\KU\cmix-upstream`
- Revision: `1d95fe95381a01442fceab585375cdec7c06922f`
- License: GNU GPL version 3
- Provenance: `E:\MIXER\KU\cmix-upstream\SOURCE_PROVENANCE.md`
- Existing snapshot checked: `E:\MIXER\KU\cmix\cmix-master`
- Modification date: 2026-08-20
- Modification notice: `third_party/cmix/MODIFICATIONS.md`

The eight canonical PPMD/LSTM donor blobs in the existing snapshot match the
authoritative commit. Raw checkout differences are CRLF conversion only.

Adaptations:

- PPMD is separated from cmix's bit projection and exposed as full-byte
  `predict[256]` plus `observe(byte)`.
- PPMD uses PROFILE_V1 order/memory and explicit allocation failure handling.
- LSTM accepts zero external mixer features while retaining byte one-hot input.
- LSTM initialization uses a private seed-controlled SplitMix64 stream instead
  of process-global `rand()`.
- LSTM prediction is cached so public prediction is side-effect free and every
  observed byte advances/trains the donor exactly once.
- cmix checkpoint glue is not exposed by the product.

## Nacrith-GPU

- Local snapshot: `E:\MIXER\KU\nacrith\Nacrith-GPU-main`
- Authoritative local checkout: `E:\MIXER\KU\nacrith-upstream`
- Revision verified against upstream:
  `ff29c42e5cfa77d7c00641880e99713644adc923`
- License: Apache License 2.0
- Provenance: `E:\MIXER\KU\nacrith-upstream.SOURCE_PROVENANCE.md`
- Reviewed files: `ngram_model.py`, `context_mixer.py`, `compressor.py`

HybridZip's C++ NGram and mixer are independent implementations of the report's
fixed algorithms. No Nacrith Python code, model wrapper, or GGUF asset is
included.
