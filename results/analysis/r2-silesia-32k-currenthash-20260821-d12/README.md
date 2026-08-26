# HybridZip R2 Current-Hash 32 KiB Portfolio Ledger

This derived ledger combines three independently validated 12-file Silesia
packages produced by the same Release binary:

- `auto`: `hybridzip-r2-auto-silesia-32k-20260821-d9`
- `predictive`: `hybridzip-r2-predictive-silesia-32k-20260821-d11`
- `lstm-compress`: `hybridzip-r2-lstm-compress-silesia-32k-20260821-d10`

All 36 rows use codec SHA-256
`DDD852EF0744740735E6D32EE0FFCB197C3C8349C0D695AD192C7CB96BF298BA`
and have `COMPLETE/PASS` byte-exact round trips.

## Files

- `mode_rows.tsv`: 36 source rows normalized for cross-mode comparison.
- `portfolio_aggregate.tsv`: one aggregate row per mode.
- `per_file_winners.tsv`: minimum archive bytes among these three modes only.

## Source CSV SHA-256

- Auto: `6A3C899F0D1E8EB3BBC120E59D961C42819D00936675F95B336B06A9D5DBF926`
- Predictive: `AD314D2AF494AFB734C8E6F44373DD272D066F5224CE69B1D1F90C7C024C85CA`
- LSTM-compress: `0943A510345469C34B36DE43426A6249FB1A887A6F11CBE8AB9C1D8526DF8E8F`

This is a current-binary three-mode ledger, not the complete 24-forced-mode
matrix. The older complete matrix remains separate because its codec hash is
different.
