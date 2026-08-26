# HybridZip R2 Family-Specific Raw Corpus Comparison

This comparison uses eight provenance-tracked donor source prefixes, exactly 32 KiB each, with the current Release binary. It contains Auto, forced predictive, and forced cmix-word-zstd results. No 64/128 KiB case was run.

- rows per mode: 8
- input bytes per mode: 262144
- Auto wins or ties: 8/8
- codec SHA-256: `DDD852EF0744740735E6D32EE0FFCB197C3C8349C0D695AD192C7CB96BF298BA`
- source manifest: `E:\MIXER\hybridzip\results\corpus\r2-family-raw-20260821\manifest.tsv`
- manifest SHA-256: `B7D5D442777A0F6B193EF390290571DF18A08F4486CA78829F4B6A86991AFD1F`

## Outputs

- comparison.tsv: per-source archive sizes, selected Auto mode, and Auto gap to the best of the three modes.
- mode_aggregate.tsv: aggregate bytes, timing, memory, and PASS counts.
- ../experiments/hybridzip-r2-family-auto-20260821-d41: Auto source package with selection/oracle logs.

All 24 rows are COMPLETE/PASS and input/decoded SHA-256 values match. This is a family-specific donor-source probe, not an independent generalization benchmark: the inputs are extracted from the donor warehouse and should be interpreted as engineering evidence only.