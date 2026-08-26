# HybridZip R2 Current-Hash Donor-Match Ledger

This derived ledger combines two explicitly scoped 32 KiB donor-match packages into one 12-file result set. No 64/128 KiB case is included.

- d20 source: `hybridzip-r2-donor-match-silesia-32k-20260821-d20` (6 rows)
- d22 source: `hybridzip-r2-donor-match-silesia-32k-20260821-d22` (6 rows)
- codec SHA-256: `DDD852EF0744740735E6D32EE0FFCB197C3C8349C0D695AD192C7CB96BF298BA`
- input: 12 Silesia files x 32 KiB = 393216 bytes
- archive bytes: 147229
- byte-exact rows: 12/12

## Verification

- d22 official validator: 6/6 PASS with explicit 32 KiB and six-file scope.
- d20 remains marked interrupted/testing because it was originally stopped after six cases; its six rows were independently revalidated here by checking all input, archive, and decoded SHA-256 values, lengths, COMPLETE, and PASS.
- Source `results.csv` SHA-256:
  - d20: `7157C310E4F944FDA3AEDFD825E4AA6CDE7F22D4567E4B5A718547EDD7112172`
  - d22: `4813647D369301D182EB86E8B0C216D75ECF8E9D4F191D6E0CC2A6308DA9B655`

## Files

- `mode_rows.tsv`: 12 normalized per-file rows.
- `portfolio_aggregate.tsv`: aggregate bytes, time, memory, and PASS count.
- `per_file_winners.tsv`: donor-match row for each file; Auto comparison is intentionally blank because this ledger is single-mode.

This is complete donor-match evidence for the current Release hash at 32 KiB only. It is not the complete 24-mode current-hash R2 portfolio.
