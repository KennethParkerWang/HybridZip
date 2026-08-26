# libFLAC Residual Donor Provenance

- Upstream project: FLAC / libFLAC
- Upstream URL: https://github.com/xiph/flac.git
- Pinned revision: `e94ff9f68b8e7dbd3e9f8b1ac18a8eca1914f181`
- Source license: BSD-3-Clause
- License evidence: `COPYING.Xiph`
- License SHA-256: `9C595EFCA136EBDAF4130124CE79AAEB64483E1DB4D39287110165BCEDC26216`
- Import date: 2026-08-20

## Imported Closure

The 18 donor files under this directory are byte-identical to the pinned donor.
They provide `fixed.c`, `lpc.c`, their direct private/public headers, and the
inlined LPC autocorrelation and bitmath dependencies. `hz_flac_donor` compiles
that closure with the upstream CMake-detected `HAVE_FSEEKO=1` and
`HAVE_LROUND=1` configuration on the current MinGW toolchain, plus
`FLAC__NO_ASM` and `FLAC__NO_DLL` to retain a portable scalar static subset.

| Path | SHA-256 |
| --- | --- |
| `src/libFLAC/fixed.c` | `D37B20832FE5C6D21E39FC10F85ECCFF0B06EBD1BEA2AE1032F9067C61D0BFFB` |
| `src/libFLAC/lpc.c` | `68CEE58E3D223A202512896773EE4A8979AA428447F8B581A32FF9F6BF130848` |
| `src/libFLAC/bitmath.c` | `4DA14FD551ED403928BE4E0D834EEB411F8655BF69C4DDBEE0BDF7C1A1635CAD` |
| `src/libFLAC/deduplication/lpc_compute_autocorrelation_intrin.c` | `68CDA07BA2B576F426134968FC41F43874882591F52B74D96337D945D3121801` |

## Integration Boundary

`src/r2/representation/flac_residual_transform.{h,cpp}` is project-owned
HZ02 framing. It exposes mode 18 for raw little-endian signed 16-bit PCM,
selecting mono, stereo-independent, or stereo mid-side planes; a plane uses a
libFLAC fixed predictor or quantized LPC predictor and a decoder-visible Rice
stream. This is not a generic FLAC container or a claim that arbitrary input
is standard FLAC audio.

`src/r2/representation/flac_donor_config.c` defines the published FLAC
`QLP_SHIFT_LEN=5` format constant required by the extracted LPC quantizer. It
is project-owned glue, not an imported donor file. The permitted integrity
evidence is `results/smoke/r2-flac-residual-32k-20260820/verification.json`.
