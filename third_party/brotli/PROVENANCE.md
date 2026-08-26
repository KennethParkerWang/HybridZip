# Brotli Text Donor Provenance

- Upstream project: Brotli
- Upstream URL: https://github.com/google/brotli
- Pinned revision: `8e10eeb3378f6c459dbaf033ca6727e9816afccb`
- Source license: MIT
- License evidence: `LICENSE`
- License SHA-256: `3D180008E36922A4E8DAEC11C34C7AF264FED5962D07924AEA928C38E8663C94`
- Retained source archive: `brotli-8e10eeb3378f6c459dbaf033ca6727e9816afccb.tar.gz`
- Source archive SHA-256: `12E2DA62A51C3D9F148297723A01654BD1CFC6D87B8FC4DAA7FA9E52E546911E`
- Import date: 2026-08-20

## Imported Closure

The donor `LICENSE` and complete `c/` source/header tree are byte-identical to
the pinned codeload source. The imported closure contains 110 files and
2,164,282 bytes. HybridZip builds the donor as three product-local static
libraries: `hz_brotli_common`, `hz_brotli_dec`, and `hz_brotli_enc`.

| Path | SHA-256 |
| --- | --- |
| `c/enc/encode.c` | `B2215A969C601A8762245B022F74BF892FAA74B166EB7FA25DE5DCCE802B285E` |
| `c/dec/decode.c` | `6B2CC010FA4AED5A65A8A0DE5F3651D3E0CD9CD78C349793DCE93844909C33D5` |
| `c/common/dictionary.c` | `0890056E4AE595EFAB6BBF3BF1FD244332E2CFAD3ADCB9F5793EE29A8F3A18C5` |

`c/enc/static_init_lazy.cc` remains byte-identical in the closure but is not
compiled: the upstream default `BROTLI_STATIC_INIT_NONE` configuration rejects
that lazy-only translation unit. The retained encoder uses the donor's default
static initialization path.

## Integration Boundary

`src/r2/representation/brotli_text_transform.{h,cpp}` is project-owned HZ02
framing. It exposes mode 19 with donor quality 11, `BROTLI_DEFAULT_WINDOW`,
and `BROTLI_MODE_TEXT`. The payload is the complete donor stream, so no
encoder-only dictionary, context, quality, or window state is required by the
decoder. The streaming decoder requires successful completion, no unconsumed
payload bytes, and exactly the outer declared raw length. The permitted smoke
evidence is `results/smoke/r2-brotli-text-32k-20260820/verification.json`.
