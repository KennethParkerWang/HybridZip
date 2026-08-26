# HybridZip R2 Current-Hash Eight-Mode Ledger

This ledger combines eight independently validated 12-file, 32 KiB Silesia
packages produced by one Release binary:

- Auto: `hybridzip-r2-auto-silesia-32k-20260821-d9`
- Predictive: `hybridzip-r2-predictive-silesia-32k-20260821-d11`
- LSTM-compress: `hybridzip-r2-lstm-compress-silesia-32k-20260821-d10`
- Stored: `hybridzip-r2-stored-silesia-32k-20260821-d13`
- zstd: `hybridzip-r2-zstd-silesia-32k-20260821-d14`
- LZMA: `hybridzip-r2-lzma-silesia-32k-20260821-d15`
- FSE: `hybridzip-r2-fse-silesia-32k-20260821-d17`
- rANS: `hybridzip-r2-rans-silesia-32k-20260821-d18`

All 96 rows use codec SHA-256
`DDD852EF0744740735E6D32EE0FFCB197C3C8349C0D695AD192C7CB96BF298BA`
and have byte-exact `COMPLETE/PASS` results.

## Files

- `mode_rows.tsv`: 96 normalized rows.
- `portfolio_aggregate.tsv`: aggregate bytes, time, memory, and PASS count.
- `per_file_winners.tsv`: minimum archive bytes among these eight modes.

## Source CSV SHA-256

- Auto: `6A3C899F0D1E8EB3BBC120E59D961C42819D00936675F95B336B06A9D5DBF926`
- Predictive: `AD314D2AF494AFB734C8E6F44373DD272D066F5224CE69B1D1F90C7C024C85CA`
- LSTM-compress: `0943A510345469C34B36DE43426A6249FB1A887A6F11CBE8AB9C1D8526DF8E8F`
- Stored: `59C1F1D6BC00B4DDB79CBDAD6F5B3A75C77CB4FF2705BEE5C4821575876E4ACC`
- zstd: `4D94ED272932BE3BD425CFF7AA32239197F024A1A3ADB1EA03F90CB49B3C44EC`
- LZMA: `7BD882A55D37CBB09ACF353FC574569DDF9CA24456114A5ECE0C09125FDAB3DC`
- FSE: `A353CED2CC55A54C7B2E23B7742193FF4BA033358274BAEF53AE30BD77311F91`
- rANS: `2BAFEA6F9B55BE531E4E8507DAB5BA4D2BE1A2102BE103DAA47FD1552C72B1DF`

The ledger covers the current-hash A6/A7 foundation controls. It is not the
complete 24-forced-mode portfolio matrix; the representation and specialist
same-hash reruns remain separate work.
