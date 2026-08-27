---
type: results-report
date: 2026-08-27
experiment_line: hybridzip-r2
round: 1
purpose: round-review
status: active
source_artifacts:
  - analysis-report.md
  - stats-appendix.md
  - figure-catalog.md
  - ../derived-final/mode_aggregate.tsv
  - ../derived-final/per_case_oracle.tsv
linked_experiments:
  - ../manifest.tsv
linked_results:
  - ../derived-final/mode_rows.tsv
---

# HybridZip R2 / Round 1 / Round Review / 2026-08-27

## Executive Summary

This round evaluated the current Release binary across Auto and all 43
decoder-visible HZ02 forced modes. The package contains 44 modes x 12 Silesia
files at a 32 KiB leading prefix, for 528 validated rows. Every row completed
an encode/decode round trip and matched input, archive, and decoded SHA-256
values.

Auto produced 99,720 complete archive bytes over 393,216 input bytes, or
2.028809 bits per input byte. The complete forced-mode oracle produced exactly
the same total, with a zero-byte gap on all 12 files. Auto selected
`paq8px-detected-sse` five times and `paq8px-generic-sse` seven times. This
supports retaining the full portfolio while promoting those two PAQ8px SSE
paths as the observed winners for this matrix.

The result is an engineering observation for one prefix size, not a global
optimality or full-corpus claim. Segment-level behavior and 64/128 KiB behavior
remain open.

## Experiment Identity and Decision Context

- Ledger ID: `hybridzip-r2-currenthash-cc6d-20260827-r2`
- Purpose: resolve whether decoder-visible Auto reaches the best complete
  archive-byte choice among the 43 forced R2 paths.
- Codec: `E:\MIXER\hybridzip\build\Release\hybridzip.exe`
- Codec SHA-256: `CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`
- Dataset: `F:\paq8px\silesia`, 12 named files, exact leading 32 KiB prefixes.
- Compatibility smoke: current Release HZ01 on a deterministic 1 KiB input,
  537-byte archive, 1,024 decoded bytes, exact SHA-256 round trip.
- Archive accounting: complete `.hz2` length, including archive header, block
  headers, CRC32 metadata, backend envelope, and payload.

The run was authorized explicitly. Existing packages were revalidated without
re-encoding; one interrupted package was resumed after removing its own stale
`.hz2.tmp` artifact. The recovery did not overwrite other packages.

## Setup and Evaluation Protocol

The HZ02 portfolio exposes modes 0 through 42. The runner executed Auto plus
each forced mode using one process per encode and decode, one repeat, one
thread, and a fixed 64 KiB HZ02 block parameter. Peak memory is the larger
sampled working set from the two processes. The primary metric is complete
archive bpb, where lower is better; timing and memory are secondary engineering
metrics.

The validation gate required all of the following for every row:

- `status=COMPLETE` and `roundtrip=PASS`;
- zero encode and decode exit codes;
- exact input, archive, and decoded artifact lengths;
- input/archive/decoded SHA-256 values matching the recorded values;
- decoded SHA-256 equal to input SHA-256;
- valid HZ02 block attribution, with every forced package recording its
  requested mode for every block;
- one codec SHA-256 across all packages.

## Main Findings

### Auto and Oracle

Auto and the complete forced oracle both totaled 99,720 archive bytes
(2.028809 bpb). Every per-file gap was exactly zero. This is stronger than a
comparison of payload-only telemetry because the comparison uses the actual
serialized archive bytes.

### Observed Winners

| Mode | Archive bytes | Weighted bpb | Oracle wins | Auto selections | Decision |
| --- | ---: | ---: | ---: | ---: | --- |
| `paq8px-detected-sse` | 99,920 | 2.032878 | 5 | 5 | retain and promote for current matrix |
| `paq8px-generic-sse` | 100,011 | 2.034729 | 7 | 7 | retain and promote for current matrix |
| `predictive` | 127,356 | 2.591064 | 0 | 0 | retain as coverage candidate |
| `brotli-text` | 128,974 | 2.623983 | 0 | 0 | retain as coverage candidate |
| `ppmd8` | 129,382 | 2.632284 | 0 | 0 | retain as coverage candidate |
| `ppmd7` | 129,614 | 2.637004 | 0 | 0 | retain as coverage candidate |
| `paq8px-apm` | 132,062 | 2.686808 | 0 | 0 | retain as coverage candidate |
| `lzma` | 133,105 | 2.708028 | 0 | 0 | retain as coverage candidate |

The complete 44-row table, including timing and memory, is
`../derived-final/mode_aggregate.tsv`. The 41 modes without an oracle win are
not deleted: this corpus has one prefix size and cannot establish that they are
redundant on future files or larger blocks.

## Statistical Validation

The unit of analysis is the Silesia file, with 12 paired file observations and
one run per mode/case. Across-file mean bpb is descriptive variation by file
type, not run-to-run uncertainty. Auto bpb and oracle bpb both have mean
2.028809, sample SD 1.232130, minimum 0.327393, median 1.959595, and maximum
4.591064. The exact Auto-oracle gap has mean, SD, minimum, median, and maximum
of zero bytes.

No p-value, confidence interval for repeated runs, effect size, normality test,
or independence-based population claim is reported. There is no repeated seed
or independent corpus split. The complete statistical boundary is recorded in
`stats-appendix.md`.

## Figure-by-Figure Interpretation

### Figure 1: Mode archive rate

`figures/figure-01-mode-bpb.pdf` and `.png` show weighted complete archive bpb
for Auto and all forced modes. The purpose is to expose the full candidate
portfolio rather than only the winner. Auto is lower than either forced SSE
aggregate because it combines the per-file winner; the two highlighted SSE
paths are the only forced modes with observed oracle wins. The figure does not
support significance or universal mode ranking.

### Figure 2: Auto versus forced oracle

`figures/figure-02-auto-oracle.pdf` and `.png` plot paired complete archive bpb
for each of the 12 files. The traces overlap exactly, making the zero gap
visible. This changes the routing decision for this matrix: no additional
decoder-visible oracle gap is present. It does not resolve segment-level
heterogeneity or unseen input distributions.

## Failure Cases, Negative Results, and Limitations

The first ledger recovery attempt was marked failed by the outer manifest due
to a PowerShell environment where `Get-FileHash` was unavailable. Artifact
inspection showed that the codec results were valid; the error was not treated
as codec evidence. The only actual runtime interruption left a zero-byte
`sao.hz2.tmp`; after the runner cleanup fix, the four missing cases completed
with exact round trips.

Most forced modes did not win an archive-byte oracle case on this matrix. That
is a negative result for this input slice, not proof of donor redundancy. The
prior random-1 KiB smoke exception for `bwt-rlt-zstd` remains a branch-gate
boundary; the current 32 KiB ledger contains valid records for every mode.

The experiment does not measure full Silesia files, 64/128 KiB prefixes,
segment-level heterogeneity, cross-platform determinism, corruption recovery,
or long-stream memory behavior. Timing is especially sensitive to the current
process and cache state.

## What Changed Our Belief

The previous uncertainty was whether the decoder-visible Auto route incurred a
measurable archive-byte penalty relative to an offline forced-mode oracle. On
this declared matrix, that uncertainty is resolved negatively: the measured
penalty is zero in every case. The evidence also narrows the current observed
winner set to the two PAQ8px SSE paths.

The evidence does not justify shrinking the R2 portfolio. The remaining modes
are still connected and byte-exact, but their coverage value is not measured by
this one prefix matrix.

## Next Actions

- Continue using Auto as the product route; keep the full 43-mode decoder-visible
  portfolio available.
- Treat `paq8px-generic-sse` and `paq8px-detected-sse` as current matrix leaders,
  while preserving all other donor paths for later coverage tests.
- Run the separately guarded segment-oracle experiment when its runtime is
  authorized, then compare segment winners with the file-level result.
- Repeat the same ledger at 64 and 128 KiB before considering any candidate
  retirement.
- Keep the report and exact TSV artifacts under the ledger directory; do not
  regenerate them against a different codec hash without a new ledger ID.

## Artifact and Reproducibility Index

- Manifest: `../manifest.tsv` (44 packages; source manifest SHA-256 is recorded
  in `../derived-final/README.md`).
- Derived rows: `../derived-final/mode_rows.tsv`.
- Per-case oracle: `../derived-final/per_case_oracle.tsv`.
- Mode aggregates: `../derived-final/mode_aggregate.tsv`.
- Auto selection view: `../derived-final/auto_selection.tsv`.
- Strict analysis: `analysis-report.md`, `stats-appendix.md`,
  `figure-catalog.md`.
- Current HZ01 smoke: `../../../../smoke/r2-final-hz01-1k-20260827/verification.json`.
- Figures: `figures/figure-01-mode-bpb.{pdf,png}` and
  `figures/figure-02-auto-oracle.{pdf,png}`.
- Reproduction commands:
  `tools/run_r2_complete_ledger.ps1 -LedgerId hybridzip-r2-currenthash-cc6d-20260827-r2 -Resume -AuthorizeRuntimeExperiment`
  and
  `tools/derive_r2_complete_ledger.ps1 -ManifestPath <manifest.tsv> -ExpectedCodecSha256 CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`.
