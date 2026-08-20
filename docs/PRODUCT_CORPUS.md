# Product Test Corpus

The first-generation product test uses nine real inputs that cover the data
classes required by the engineering report. Large sources are sampled by an
exact leading-byte prefix so the fixed PROFILE_V1 implementation can complete
the full encode/decode and baseline matrix in a practical time.

| Case | Type | Source | Selection |
|---|---|---|---:|
| `plain-text` | plain text | `E:\MIXER\silesia\dickens` | first 16 KiB |
| `source-code` | source code | `E:\MIXER\KU\cmix-upstream\src\mixer\lstm.cpp` | complete file |
| `json-xml` | JSON/XML | `E:\MIXER\silesia\xml` | first 16 KiB |
| `binary-executable` | binary executable | `build\Release\hybridzip.exe` | first 16 KiB |
| `database-records` | database/binary records | `E:\MIXER\silesia\osdb` | first 16 KiB |
| `image-xray` | image | `E:\MIXER\silesia\x-ray` | first 16 KiB |
| `audio-media` | audio/media | `E:\MIXER\KU\product-corpus\sample.wav` | first 16 KiB |
| `compressed-high-entropy` | already compressed/high entropy | Phase 5 HZ01 archive | first 16 KiB |
| `large-mixed` | large mixed binary | `E:\MIXER\silesia\mozilla` | first 128 KiB |

The generated input bytes and their SHA-256 values are written to
`results/product_manifest.tsv`. The downloaded audio provenance is recorded in
`E:\MIXER\KU\product-corpus\SOURCE_PROVENANCE.md`.

The separate Experiment Ledger run covers all 12 public Silesia members at
32, 64, and 128 KiB. That 36-case run is the larger public heterogeneous corpus
check requested by the report.
