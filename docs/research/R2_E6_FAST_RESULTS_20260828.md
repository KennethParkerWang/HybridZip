# HybridZip R2 E6 Fast Results

## Result

The historical mode-2 `fast` policy passed its CPU floor on the frozen Tier-A
Silesia matrix. Fast K=4 and the Mode-43 extension are now implemented, but
this result is not acceptance evidence for that changed policy.

## Protocol

- Package: `results/experiments/hybridzip-r2-e6-fast-full-20260828-retry1/`
- Input: all 12 Silesia files, leading 32/64/128 KiB prefixes.
- Internal block sizes: 32, 64, and 128 KiB.
- Policy: `fast`, existing HZ02 zstd mode 2, level cap 3.
- Timing: one warmup plus three retained repeats.
- Executable SHA-256:
  `E65526F9DFF3F93844E004D63C7B2A4E4F219B5EAB3F1B3D3ABCF0B301F65003`.

## Checks

- 12 child packages and 432 total encode/decode rows completed.
- 108 warmup rows are preserved in `matrix_rows.csv` and excluded from timing
  summaries; 324 rows are retained.
- Every one of 432 rows is `COMPLETE/PASS` with byte-exact reconstruction.
- The experiment's 36 input groups have one consistent SHA-256 per group.

## Throughput Gate

The threshold is 0.16 MB/s for both encode and decode in every summary cell.

| Minimum observed aggregate rate | Value |
| --- | ---: |
| Encode | 0.6977 MB/s |
| Decode | 0.6476 MB/s |

All nine `(input scope, internal block size)` cells pass. Across those cells,
encode P95 is 57.78–60.65 ms, decode P95 is 62.56–66.69 ms, and peak sampled
RAM is 5.00–5.42 MiB. The authoritative per-cell values are in
`summary_rows.csv`; aggregate status is in `summary.json`.

## Boundary

The installed zstd donor identifies as 1.6.0, while the external decision
specified a future pinned 1.5.7 import. The result therefore establishes the
historical mode-2 Fast baseline only. It does not prove the now-implemented
Fast K=4 candidate policy, `MODE_FAST_EXT_V1`, transform selection under the
corpus matrix, block parallelism, or the 8–10 MB/s GPU target.
