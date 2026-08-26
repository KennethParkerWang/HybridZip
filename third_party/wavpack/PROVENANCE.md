# WavPack Donor Provenance

- Upstream project: WavPack
- Upstream URL: https://github.com/dbry/WavPack
- Pinned revision: `eccf998c7acce58e18dedd354e6b025728dcf6da`
- Source license: BSD-3-Clause
- License evidence: `COPYING`
- License SHA-256: `66182C49C182998173188B0431D4DE653274C9F43D391A3EF2489C69952B1A6A`
- Import date: 2026-08-21

## Imported Closure

The 24 donor files under this directory are byte-identical to the pinned
checkout. The 217-file checkout remains available under `KU` for study; this
distribution copies only the complete lossless memory pack/unpack closure:

| Path | SHA-256 |
| --- | --- |
| `COPYING` | `66182C49C182998173188B0431D4DE653274C9F43D391A3EF2489C69952B1A6A` |
| `include/wavpack.h` | `A86EF1029CBD3A99BD133CF8F6705A31B220295377A4AD4504F492F185DC0291` |
| `src/common_utils.c` | `F59338FA9457B26C5E9BDC28B7B5CB975EEC8E3316E91018652AC7689B2228B9` |
| `src/decorr_tables.h` | `A808AD448B8D6D2B205C8FFFF735B88622AB7973D8215B092D04B110E9CA50E8` |
| `src/decorr_utils.c` | `8243CAC5E0702FA712EAE6AFFC83705FAD86FE078F6A2B823097170016E64D01` |
| `src/entropy_utils.c` | `2B1DA5864B04672D3240C51245BA429159CAED471D5152E966066AEC174620E6` |
| `src/extra1.c` | `7AB2CB1DEE23B858DBB8540AE5FF8DDA9013AB6F3732C8342838E20C0CA9FBD8` |
| `src/extra2.c` | `A09B76884FC8A8B231C7E7F72524F4908C38CA55D53452CD372E1CFDE7CEAB9E` |
| `src/open_raw.c` | `3B263A57CDB2A4B9526DDEB16E20382B1A253D55FCA1451485611F10C904F26E` |
| `src/open_utils.c` | `B72D3ED1995EB90E0475ED963DE31B37BD2E149132448E571B6FA38D72A2EBE4` |
| `src/pack_dns.c` | `9A0A78F6978D1D21394CB824EFB70C024D4A54E7E2F42BD64CDC12F5D3BE86AD` |
| `src/pack_floats.c` | `FE75EE6F2ED6EAF977D06957DFDFC6088710231F63756A83BD57E36F2DF3BC84` |
| `src/pack_utils.c` | `A80F541492DEA4992EA3D7F9F87F0956C082BBC9DA929D16184D750009301612` |
| `src/pack.c` | `3F477518E36F81367C9F451BC818E6F7A0F1FB181E4509F3D3624E6D7CBDB5D0` |
| `src/read_words.c` | `7255A216E62CD97E6C793A6829FD1AA0961F5F8A88CB14A6EC53E00A66F20513` |
| `src/tag_utils.c` | `E63CCCF9D1C56DD196E6A354030C551E407210B4DCE601ACF0BE081A2CF51047` |
| `src/tags.c` | `53D900A6A966D24A679E1D9CCBF70F54B531ECCA5E7CBD124484339B2A22B241` |
| `src/unpack_floats.c` | `A8163A593C31358F33AE1DD319CE307368657A53F9B8E1EA260884ABF1E2EF24` |
| `src/unpack_seek.c` | `1D1982848099C25616F3CB0ED0FB91621DB031F27E1B7A09C97B237E2AFAF510` |
| `src/unpack_utils.c` | `73E777D5547FDE0239EC0BB8E10EB96A7BD18AFCE67C0BDDE5968DA286D17396` |
| `src/unpack.c` | `211E3C41CD9072471AD4EF30902243CF6751303849B379CD4A63A7DBB813C1F3` |
| `src/wavpack_local.h` | `7EBFD1CECFCFA04CF278C54F2AC8E98DD2DDC26D8862225623863C65AD0FCD5A` |
| `src/wavpack_version.h` | `BA7B067552849E75E3623DCCF64BAA89A20B972B25D72DD2D89E1E0E507C27A7` |
| `src/write_words.c` | `BDCB4939371B5B73D601BF740F2676A30B075D5315DD4C0CF17DB2468904B086` |

## Integration Boundary

`src/r2/entropy/wavpack_backend.{h,cpp}` is project-owned HZ02 framing. It
tries deterministic 8/16/24/32-bit mono/stereo PCM profiles, stores the chosen
profile and any incomplete-frame raw tail in an `HZW1` payload header, and
invokes the complete donor lossless packer and unpacker through memory
callbacks. The archive decoder verifies the lossless flag, sample width,
channel count, frame count, exact stream consumption, decoded size, and outer
HZ02 CRC32. The adapter is a raw-byte candidate, not a standard `.wv` file
container or a claim that arbitrary WavPack metadata/tag features are present.

HZ02 mode 38 uses raw transform `0` and entropy ID `21`. The backend payload
bound is `4 * uncompressed_size + 16 + 4096` bytes. The evidence gate is the
single forced 1 KiB encode/decode in
`results/smoke/r2-wavpack-1k-20260821/verification.json`.
