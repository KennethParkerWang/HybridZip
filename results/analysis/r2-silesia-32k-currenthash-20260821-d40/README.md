# HybridZip R2 Current-Hash Complete Portfolio Ledger

This ledger combines the current Release binary's Auto path and all 24 forced R2 modes on the 12 Silesia files at exactly 32 KiB per file. It contains 25 modes and 300 independently byte-exact rows. No 64 KiB or 128 KiB case is included.

- input bytes: 393216
- codec SHA-256: `DDD852EF0744740735E6D32EE0FFCB197C3C8349C0D695AD192C7CB96BF298BA`
- normalized rows: 300
- PASS rows: 300

## Source packages

- auto: `hybridzip-r2-auto-silesia-32k-20260821-d9` (12 rows, CSV SHA-256 `6A3C899F0D1E8EB3BBC120E59D961C42819D00936675F95B336B06A9D5DBF926`)
- predictive: `hybridzip-r2-predictive-silesia-32k-20260821-d11` (12 rows, CSV SHA-256 `AD314D2AF494AFB734C8E6F44373DD272D066F5224CE69B1D1F90C7C024C85CA`)
- lstm-compress: `hybridzip-r2-lstm-compress-silesia-32k-20260821-d10` (12 rows, CSV SHA-256 `0943A510345469C34B36DE43426A6249FB1A887A6F11CBE8AB9C1D8526DF8E8F`)
- stored: `hybridzip-r2-stored-silesia-32k-20260821-d13` (12 rows, CSV SHA-256 `59C1F1D6BC00B4DDB79CBDAD6F5B3A75C77CB4FF2705BEE5C4821575876E4ACC`)
- zstd: `hybridzip-r2-zstd-silesia-32k-20260821-d14` (12 rows, CSV SHA-256 `4D94ED272932BE3BD425CFF7AA32239197F024A1A3ADB1EA03F90CB49B3C44EC`)
- lzma: `hybridzip-r2-lzma-silesia-32k-20260821-d15` (12 rows, CSV SHA-256 `7BD882A55D37CBB09ACF353FC574569DDF9CA24456114A5ECE0C09125FDAB3DC`)
- fse: `hybridzip-r2-fse-silesia-32k-20260821-d17` (12 rows, CSV SHA-256 `A353CED2CC55A54C7B2E23B7742193FF4BA033358274BAEF53AE30BD77311F91`)
- rans: `hybridzip-r2-rans-silesia-32k-20260821-d18` (12 rows, CSV SHA-256 `2BAFEA6F9B55BE531E4E8507DAB5BA4D2BE1A2102BE103DAA47FD1552C72B1DF`)
- donor-match: `hybridzip-r2-donor-match-silesia-32k-20260821-d20` (6 rows, CSV SHA-256 `7157C310E4F944FDA3AEDFD825E4AA6CDE7F22D4567E4B5A718547EDD7112172`)
- donor-match: `hybridzip-r2-donor-match-silesia-32k-20260821-d22` (6 rows, CSV SHA-256 `4813647D369301D182EB86E8B0C216D75ECF8E9D4F191D6E0CC2A6308DA9B655`)
- bwt-zstd: `hybridzip-r2-bwt-zstd-silesia-32k-20260821-d24` (12 rows, CSV SHA-256 `7A816AA2CCDD0A46A166C89F98AADF8DCE8452059E8E446F019C297F66D0D741`)
- bwt-mtf-zstd: `hybridzip-r2-bwt-mtf-zstd-silesia-32k-20260821-d25` (12 rows, CSV SHA-256 `9CB7BE8536385B7342715F36DD1A0CA279DF65645B6B76BD79546176BE276309`)
- bwt-rlt-zstd: `hybridzip-r2-bwt-rlt-zstd-silesia-32k-20260821-d26` (12 rows, CSV SHA-256 `1ECC0C2B4A6704A9B7312FF0CA7D27482EE69C81337E7DB6F2FB10220068172D`)
- x86-bcj-zstd: `hybridzip-r2-x86-bcj-zstd-silesia-32k-20260821-d27` (12 rows, CSV SHA-256 `7B17CE21D7818B93B31F61717F139B89C4682379105600F5152BE3AAFB2A3FE9`)
- shuffle-zstd: `hybridzip-r2-shuffle-zstd-silesia-32k-20260821-d28` (12 rows, CSV SHA-256 `F5605DBC41CAB2492BCD182A3F506EF3030E4924DCD462F92EA01417ABA498F4`)
- bitshuffle-zstd: `hybridzip-r2-bitshuffle-zstd-silesia-32k-20260821-d29` (12 rows, CSV SHA-256 `29A2844FA4C307D4E9265CC56E1C56CAC5F436F13ACE7D158668A9893AC18308`)
- delta-zstd: `hybridzip-r2-delta-zstd-silesia-32k-20260821-d30` (12 rows, CSV SHA-256 `B31279410BDB2CB47A30BE2A779C5EFD3F3FD38FCAFEBC580CE94A563355E80E`)
- fastpfor: `hybridzip-r2-fastpfor-silesia-32k-20260821-d31` (12 rows, CSV SHA-256 `178E38E719820F18D951BABF6426D43F39194513743678F6F72C87C52AB3175A`)
- bcj2-zstd: `hybridzip-r2-bcj2-zstd-silesia-32k-20260821-d32` (12 rows, CSV SHA-256 `2DF146BE493ABF75D4E09F85F50C62A2F6FC8CF5E22E11BA0DB1D533E8B5A8F8`)
- record-transpose-zstd: `hybridzip-r2-record-transpose-zstd-silesia-32k-20260821-d33` (12 rows, CSV SHA-256 `AFC0E93DA9F49A05033365B2924B2A7308B9EEFEC959F649629F8932743E6F54`)
- jpegls: `hybridzip-r2-jpegls-silesia-32k-20260821-d34` (12 rows, CSV SHA-256 `BA1187F00F373179A1AA5B4DA9CD11ED53ABFE650FE20614E34B145D3AB22AAC`)
- flac-residual: `hybridzip-r2-flac-residual-silesia-32k-20260821-d35` (12 rows, CSV SHA-256 `59768743998C5488EEBC597B7DE8A351B5D1973CC3DF2B141745F86A4F32323D`)
- brotli-text: `hybridzip-r2-brotli-text-silesia-32k-20260821-d36` (12 rows, CSV SHA-256 `0B7289824437119C9BD4D8A1DC7D7707A4A369201C81F7C7A0C8512887A1A5BF`)
- cmix-word-zstd: `hybridzip-r2-cmix-word-zstd-silesia-32k-20260821-d37` (12 rows, CSV SHA-256 `D7BFCF86729E723849FFF97CC91086CAD9927A37ECA9C59FBE9AF4570E0D6EBA`)
- neural-lstm: `hybridzip-r2-neural-lstm-silesia-32k-20260821-d38` (12 rows, CSV SHA-256 `6DF7AB54FA81757F02907C68FD0160C842ACF606077098BE80E299BB60CD5913`)
- shared-neural-lstm: `hybridzip-r2-shared-neural-lstm-silesia-32k-20260821-d39` (12 rows, CSV SHA-256 `B287E1B174544B42738D45C1F30CCF76D3D739B165E3EFDDA67397A86158B70C`)

The donor-match mode combines its six-row d20 package and six-row d22 package. d20 remains marked testing because it was intentionally stopped; its six rows are revalidated here at artifact level. d22 passed its explicit six-file validator. Historical d6 packages are not used in this ledger.

## Files

- mode_rows.tsv: 300 normalized per-file rows.
- portfolio_aggregate.tsv: one aggregate row per mode.
- per_file_winners.tsv: minimum archive bytes and Auto gap for each file.
- source_manifest.tsv: source package row counts and CSV hashes.

This ledger is the current-hash 32 KiB portfolio evidence. It does not by itself claim broader corpus generalization, repeated-run statistics, or 64/128 KiB performance.
