# D40 Detailed Result Table

Source: `r2-silesia-32k-currenthash-20260821-d40`.
This is a derived view; the source TSV files are unchanged.

- Modes: 25 (Auto + 24 forced)
- Files: 12 Silesia files, 32 KiB each
- Rows: 300; PASS: 300
- Codec SHA-256: `DDD852EF0744740735E6D32EE0FFCB197C3C8349C0D695AD192C7CB96BF298BA`

## Mode Summary

| Mode | Rows | Archive bytes | Ratio | Bits/byte | Encode s | Decode s | Peak RAM MiB | PASS |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| auto | 12 | 121293 | 0.308464 | 2.467712 | 928.032 | 121.059 | 130.363 | 12 |
| predictive | 12 | 127356 | 0.323883 | 2.591064 | 224.295 | 225.038 | 129.887 | 12 |
| lstm-compress | 12 | 143803 | 0.365710 | 2.925680 | 157.703 | 158.737 | 23.492 | 12 |
| stored | 12 | 393936 | 1.001831 | 8.014648 | 0.587 | 0.464 | 3.969 | 12 |
| zstd | 12 | 143871 | 0.365883 | 2.927063 | 0.630 | 0.505 | 4.508 | 12 |
| lzma | 12 | 133105 | 0.338504 | 2.708028 | 0.599 | 0.486 | 4.551 | 12 |
| fse | 12 | 255744 | 0.650391 | 5.203125 | 0.586 | 0.460 | 4.004 | 12 |
| rans | 12 | 259307 | 0.659452 | 5.275614 | 0.568 | 0.462 | 4.004 | 12 |
| donor-match | 12 | 147229 | 0.374423 | 2.995382 | 227.176 | 227.787 | 129.906 | 12 |
| bwt-zstd | 12 | 153068 | 0.389272 | 3.114176 | 0.658 | 0.540 | 4.695 | 12 |
| bwt-mtf-zstd | 12 | 147588 | 0.375336 | 3.002686 | 0.706 | 0.536 | 4.695 | 12 |
| bwt-rlt-zstd | 12 | 156142 | 0.397090 | 3.176717 | 0.634 | 0.461 | 4.668 | 12 |
| x86-bcj-zstd | 12 | 142233 | 0.361717 | 2.893738 | 0.631 | 0.483 | 4.531 | 12 |
| shuffle-zstd | 12 | 166336 | 0.423014 | 3.384115 | 0.713 | 0.497 | 4.574 | 12 |
| bitshuffle-zstd | 12 | 255337 | 0.649356 | 5.194845 | 0.673 | 0.455 | 4.551 | 12 |
| delta-zstd | 12 | 165252 | 0.420258 | 3.362061 | 0.802 | 0.459 | 4.559 | 12 |
| fastpfor | 12 | 358024 | 0.910502 | 7.284017 | 0.560 | 0.463 | 3.863 | 12 |
| bcj2-zstd | 12 | 143959 | 0.366107 | 2.928853 | 0.587 | 0.485 | 5.262 | 12 |
| record-transpose-zstd | 12 | 210187 | 0.534533 | 4.276265 | 0.631 | 0.489 | 4.551 | 12 |
| jpegls | 12 | 322772 | 0.820852 | 6.566813 | 0.613 | 0.552 | 3.715 | 12 |
| flac-residual | 12 | 356741 | 0.907239 | 7.257914 | 0.743 | 0.584 | 4.535 | 12 |
| brotli-text | 12 | 128974 | 0.327998 | 2.623983 | 0.993 | 0.517 | 6.488 | 12 |
| cmix-word-zstd | 12 | 150572 | 0.382924 | 3.063395 | 0.858 | 0.678 | 7.855 | 12 |
| neural-lstm | 12 | 162028 | 0.412059 | 3.296468 | 218.904 | 217.972 | 120.816 | 12 |
| shared-neural-lstm | 12 | 161041 | 0.409548 | 3.276388 | 219.419 | 218.847 | 120.828 | 12 |

## Per-file Winners

| File | Winner mode(s) | Winner archive bytes | Auto archive bytes | Auto gap bytes |
| --- | --- | ---: | ---: | ---: |
| dickens | auto,cmix-word-zstd | 10890 | 10890 | 0 |
| mozilla | auto,lzma | 8601 | 8601 | 0 |
| mr | auto,lstm-compress | 6266 | 6266 | 0 |
| nci | auto,brotli-text | 2366 | 2366 | 0 |
| ooffice | auto,x86-bcj-zstd | 11362 | 11362 | 0 |
| osdb | auto,predictive | 13929 | 13929 | 0 |
| reymont | auto,predictive | 7263 | 7263 | 0 |
| samba | auto,cmix-word-zstd | 9020 | 9020 | 0 |
| sao | auto,lstm-compress | 21710 | 21710 | 0 |
| webster | auto,predictive | 10867 | 10867 | 0 |
| x-ray | auto,lstm-compress | 16668 | 16668 | 0 |
| xml | auto,predictive | 2351 | 2351 | 0 |

## Source Packages

| Mode | Package | Rows | Results CSV SHA-256 |
| --- | --- | ---: | --- |
| auto | `hybridzip-r2-auto-silesia-32k-20260821-d9` | 12 | `6A3C899F0D1E8EB3BBC120E59D961C42819D00936675F95B336B06A9D5DBF926` |
| predictive | `hybridzip-r2-predictive-silesia-32k-20260821-d11` | 12 | `AD314D2AF494AFB734C8E6F44373DD272D066F5224CE69B1D1F90C7C024C85CA` |
| lstm-compress | `hybridzip-r2-lstm-compress-silesia-32k-20260821-d10` | 12 | `0943A510345469C34B36DE43426A6249FB1A887A6F11CBE8AB9C1D8526DF8E8F` |
| stored | `hybridzip-r2-stored-silesia-32k-20260821-d13` | 12 | `59C1F1D6BC00B4DDB79CBDAD6F5B3A75C77CB4FF2705BEE5C4821575876E4ACC` |
| zstd | `hybridzip-r2-zstd-silesia-32k-20260821-d14` | 12 | `4D94ED272932BE3BD425CFF7AA32239197F024A1A3ADB1EA03F90CB49B3C44EC` |
| lzma | `hybridzip-r2-lzma-silesia-32k-20260821-d15` | 12 | `7BD882A55D37CBB09ACF353FC574569DDF9CA24456114A5ECE0C09125FDAB3DC` |
| fse | `hybridzip-r2-fse-silesia-32k-20260821-d17` | 12 | `A353CED2CC55A54C7B2E23B7742193FF4BA033358274BAEF53AE30BD77311F91` |
| rans | `hybridzip-r2-rans-silesia-32k-20260821-d18` | 12 | `2BAFEA6F9B55BE531E4E8507DAB5BA4D2BE1A2102BE103DAA47FD1552C72B1DF` |
| donor-match | `hybridzip-r2-donor-match-silesia-32k-20260821-d20` | 6 | `7157C310E4F944FDA3AEDFD825E4AA6CDE7F22D4567E4B5A718547EDD7112172` |
| donor-match | `hybridzip-r2-donor-match-silesia-32k-20260821-d22` | 6 | `4813647D369301D182EB86E8B0C216D75ECF8E9D4F191D6E0CC2A6308DA9B655` |
| bwt-zstd | `hybridzip-r2-bwt-zstd-silesia-32k-20260821-d24` | 12 | `7A816AA2CCDD0A46A166C89F98AADF8DCE8452059E8E446F019C297F66D0D741` |
| bwt-mtf-zstd | `hybridzip-r2-bwt-mtf-zstd-silesia-32k-20260821-d25` | 12 | `9CB7BE8536385B7342715F36DD1A0CA279DF65645B6B76BD79546176BE276309` |
| bwt-rlt-zstd | `hybridzip-r2-bwt-rlt-zstd-silesia-32k-20260821-d26` | 12 | `1ECC0C2B4A6704A9B7312FF0CA7D27482EE69C81337E7DB6F2FB10220068172D` |
| x86-bcj-zstd | `hybridzip-r2-x86-bcj-zstd-silesia-32k-20260821-d27` | 12 | `7B17CE21D7818B93B31F61717F139B89C4682379105600F5152BE3AAFB2A3FE9` |
| shuffle-zstd | `hybridzip-r2-shuffle-zstd-silesia-32k-20260821-d28` | 12 | `F5605DBC41CAB2492BCD182A3F506EF3030E4924DCD462F92EA01417ABA498F4` |
| bitshuffle-zstd | `hybridzip-r2-bitshuffle-zstd-silesia-32k-20260821-d29` | 12 | `29A2844FA4C307D4E9265CC56E1C56CAC5F436F13ACE7D158668A9893AC18308` |
| delta-zstd | `hybridzip-r2-delta-zstd-silesia-32k-20260821-d30` | 12 | `B31279410BDB2CB47A30BE2A779C5EFD3F3FD38FCAFEBC580CE94A563355E80E` |
| fastpfor | `hybridzip-r2-fastpfor-silesia-32k-20260821-d31` | 12 | `178E38E719820F18D951BABF6426D43F39194513743678F6F72C87C52AB3175A` |
| bcj2-zstd | `hybridzip-r2-bcj2-zstd-silesia-32k-20260821-d32` | 12 | `2DF146BE493ABF75D4E09F85F50C62A2F6FC8CF5E22E11BA0DB1D533E8B5A8F8` |
| record-transpose-zstd | `hybridzip-r2-record-transpose-zstd-silesia-32k-20260821-d33` | 12 | `AFC0E93DA9F49A05033365B2924B2A7308B9EEFEC959F649629F8932743E6F54` |
| jpegls | `hybridzip-r2-jpegls-silesia-32k-20260821-d34` | 12 | `BA1187F00F373179A1AA5B4DA9CD11ED53ABFE650FE20614E34B145D3AB22AAC` |
| flac-residual | `hybridzip-r2-flac-residual-silesia-32k-20260821-d35` | 12 | `59768743998C5488EEBC597B7DE8A351B5D1973CC3DF2B141745F86A4F32323D` |
| brotli-text | `hybridzip-r2-brotli-text-silesia-32k-20260821-d36` | 12 | `0B7289824437119C9BD4D8A1DC7D7707A4A369201C81F7C7A0C8512887A1A5BF` |
| cmix-word-zstd | `hybridzip-r2-cmix-word-zstd-silesia-32k-20260821-d37` | 12 | `D7BFCF86729E723849FFF97CC91086CAD9927A37ECA9C59FBE9AF4570E0D6EBA` |
| neural-lstm | `hybridzip-r2-neural-lstm-silesia-32k-20260821-d38` | 12 | `6DF7AB54FA81757F02907C68FD0160C842ACF606077098BE80E299BB60CD5913` |
| shared-neural-lstm | `hybridzip-r2-shared-neural-lstm-silesia-32k-20260821-d39` | 12 | `B287E1B174544B42738D45C1F30CCF76D3D739B165E3EFDDA67397A86158B70C` |

## Full Per-file Matrix

The complete 300-row matrix is in `mode_rows.tsv` and the XLSX sheet `Detailed Rows`; this report keeps the overview compact for review.

| Mode | File | Archive bytes | Ratio | Encode s | Decode s | Peak RAM MiB | Result | Block type |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| auto | dickens | 10890 | 0.332336 | 84.862 | 0.067 | 130.125 | PASS | cmix-word-zstd=1 |
| auto | mozilla | 8601 | 0.262482 | 89.327 | 0.038 | 130.289 | PASS | lzma=1 |
| auto | mr | 6266 | 0.191223 | 50.925 | 11.629 | 130.320 | PASS | lstm-compress=1 |
| auto | nci | 2366 | 0.072205 | 85.204 | 0.038 | 130.281 | PASS | brotli-text=1 |
| auto | ooffice | 11362 | 0.346741 | 38.254 | 0.038 | 130.215 | PASS | x86-bcj-zstd=1 |
| auto | osdb | 13929 | 0.425079 | 88.404 | 18.940 | 130.246 | PASS | predictive=1 |
| auto | reymont | 7263 | 0.221649 | 83.864 | 18.161 | 130.086 | PASS | predictive=1 |
| auto | samba | 9020 | 0.275269 | 84.500 | 0.051 | 130.176 | PASS | cmix-word-zstd=1 |
| auto | sao | 21710 | 0.662537 | 89.200 | 16.581 | 130.141 | PASS | lstm-compress=1 |
| auto | webster | 10867 | 0.331635 | 88.602 | 18.746 | 130.133 | PASS | predictive=1 |
| auto | x-ray | 16668 | 0.508667 | 54.457 | 17.564 | 130.363 | PASS | lstm-compress=1 |
| auto | xml | 2351 | 0.071747 | 90.434 | 19.204 | 130.301 | PASS | predictive=1 |
| predictive | dickens | 11052 | 0.337280 | 18.100 | 17.976 | 129.762 | PASS | predictive=1 |
| predictive | mozilla | 9487 | 0.289520 | 18.459 | 18.620 | 129.512 | PASS | predictive=1 |
| predictive | mr | 6507 | 0.198578 | 18.145 | 18.232 | 129.512 | PASS | predictive=1 |
| predictive | nci | 2519 | 0.076874 | 18.489 | 18.724 | 129.500 | PASS | predictive=1 |
| predictive | ooffice | 12453 | 0.380035 | 19.256 | 19.425 | 129.426 | PASS | predictive=1 |
| predictive | osdb | 13929 | 0.425079 | 19.127 | 19.355 | 129.883 | PASS | predictive=1 |
| predictive | reymont | 7263 | 0.221649 | 18.584 | 18.584 | 129.520 | PASS | predictive=1 |
| predictive | samba | 9272 | 0.282959 | 18.653 | 18.661 | 129.887 | PASS | predictive=1 |
| predictive | sao | 23038 | 0.703064 | 18.961 | 18.792 | 129.863 | PASS | predictive=1 |
| predictive | webster | 10867 | 0.331635 | 18.727 | 18.890 | 129.520 | PASS | predictive=1 |
| predictive | x-ray | 18618 | 0.568176 | 18.927 | 19.153 | 129.887 | PASS | predictive=1 |
| predictive | xml | 2351 | 0.071747 | 18.869 | 18.628 | 129.496 | PASS | predictive=1 |
| lstm-compress | dickens | 13353 | 0.407501 | 12.128 | 12.184 | 23.418 | PASS | lstm-compress=1 |
| lstm-compress | mozilla | 9701 | 0.296051 | 13.402 | 13.076 | 23.410 | PASS | lstm-compress=1 |
| lstm-compress | mr | 6266 | 0.191223 | 12.134 | 12.604 | 23.309 | PASS | lstm-compress=1 |
| lstm-compress | nci | 3743 | 0.114227 | 12.143 | 12.113 | 23.000 | PASS | lstm-compress=1 |
| lstm-compress | ooffice | 12092 | 0.369019 | 13.703 | 13.772 | 23.414 | PASS | lstm-compress=1 |
| lstm-compress | osdb | 20835 | 0.635834 | 13.840 | 13.778 | 23.492 | PASS | lstm-compress=1 |
| lstm-compress | reymont | 8881 | 0.271027 | 11.995 | 12.153 | 23.402 | PASS | lstm-compress=1 |
| lstm-compress | samba | 12334 | 0.376404 | 12.225 | 12.294 | 23.453 | PASS | lstm-compress=1 |
| lstm-compress | sao | 21710 | 0.662537 | 17.116 | 17.116 | 23.086 | PASS | lstm-compress=1 |
| lstm-compress | webster | 13379 | 0.408295 | 11.938 | 11.984 | 23.422 | PASS | lstm-compress=1 |
| lstm-compress | x-ray | 16668 | 0.508667 | 15.441 | 15.994 | 23.469 | PASS | lstm-compress=1 |
| lstm-compress | xml | 4841 | 0.147736 | 11.639 | 11.669 | 23.012 | PASS | lstm-compress=1 |
| stored | dickens | 32828 | 1.001831 | 0.050 | 0.048 | 2.355 | PASS | stored=1 |
| stored | mozilla | 32828 | 1.001831 | 0.048 | 0.037 | 3.969 | PASS | stored=1 |
| stored | mr | 32828 | 1.001831 | 0.044 | 0.038 | 2.012 | PASS | stored=1 |
| stored | nci | 32828 | 1.001831 | 0.062 | 0.038 | 3.535 | PASS | stored=1 |
| stored | ooffice | 32828 | 1.001831 | 0.052 | 0.038 | 2.004 | PASS | stored=1 |
| stored | osdb | 32828 | 1.001831 | 0.045 | 0.037 | 3.355 | PASS | stored=1 |
| stored | reymont | 32828 | 1.001831 | 0.045 | 0.038 | 3.543 | PASS | stored=1 |
| stored | samba | 32828 | 1.001831 | 0.035 | 0.038 | 2.004 | PASS | stored=1 |
| stored | sao | 32828 | 1.001831 | 0.050 | 0.038 | 2.004 | PASS | stored=1 |
| stored | webster | 32828 | 1.001831 | 0.042 | 0.037 | 2.004 | PASS | stored=1 |
| stored | x-ray | 32828 | 1.001831 | 0.048 | 0.038 | 3.918 | PASS | stored=1 |
| stored | xml | 32828 | 1.001831 | 0.065 | 0.038 | 3.953 | PASS | stored=1 |
| zstd | dickens | 13216 | 0.403320 | 0.060 | 0.049 | 4.492 | PASS | zstd=1 |
| zstd | mozilla | 9548 | 0.291382 | 0.056 | 0.038 | 2.004 | PASS | zstd=1 |
| zstd | mr | 7957 | 0.242828 | 0.049 | 0.050 | 4.027 | PASS | zstd=1 |
| zstd | nci | 2480 | 0.075684 | 0.064 | 0.038 | 4.500 | PASS | zstd=1 |
| zstd | ooffice | 13019 | 0.397308 | 0.050 | 0.037 | 4.398 | PASS | zstd=1 |
| zstd | osdb | 15144 | 0.462158 | 0.043 | 0.038 | 2.004 | PASS | zstd=1 |
| zstd | reymont | 9108 | 0.277954 | 0.064 | 0.053 | 4.395 | PASS | zstd=1 |
| zstd | samba | 11027 | 0.336517 | 0.052 | 0.035 | 4.469 | PASS | zstd=1 |
| zstd | sao | 24423 | 0.745331 | 0.048 | 0.038 | 3.898 | PASS | zstd=1 |
| zstd | webster | 12772 | 0.389771 | 0.046 | 0.038 | 4.254 | PASS | zstd=1 |
| zstd | x-ray | 22464 | 0.685547 | 0.049 | 0.038 | 4.508 | PASS | zstd=1 |
| zstd | xml | 2713 | 0.082794 | 0.049 | 0.052 | 4.379 | PASS | zstd=1 |
| lzma | dickens | 13027 | 0.397552 | 0.053 | 0.045 | 2.004 | PASS | lzma=1 |
| lzma | mozilla | 8601 | 0.262482 | 0.055 | 0.038 | 4.543 | PASS | lzma=1 |
| lzma | mr | 6731 | 0.205414 | 0.050 | 0.049 | 3.590 | PASS | lzma=1 |
| lzma | nci | 2495 | 0.076141 | 0.049 | 0.037 | 4.352 | PASS | lzma=1 |
| lzma | ooffice | 12372 | 0.377563 | 0.048 | 0.038 | 4.426 | PASS | lzma=1 |
| lzma | osdb | 14483 | 0.441986 | 0.056 | 0.037 | 4.547 | PASS | lzma=1 |
| lzma | reymont | 8810 | 0.268860 | 0.050 | 0.053 | 4.449 | PASS | lzma=1 |
| lzma | samba | 10865 | 0.331573 | 0.049 | 0.039 | 4.449 | PASS | lzma=1 |
| lzma | sao | 22279 | 0.679901 | 0.047 | 0.036 | 4.359 | PASS | lzma=1 |
| lzma | webster | 12595 | 0.384369 | 0.045 | 0.038 | 2.004 | PASS | lzma=1 |
| lzma | x-ray | 18233 | 0.556427 | 0.051 | 0.038 | 4.551 | PASS | lzma=1 |
| lzma | xml | 2614 | 0.079773 | 0.047 | 0.037 | 2.480 | PASS | lzma=1 |
| fse | dickens | 19344 | 0.590332 | 0.059 | 0.048 | 4.004 | PASS | fse=1 |
| fse | mozilla | 21274 | 0.649231 | 0.037 | 0.038 | 2.004 | PASS | fse=1 |
| fse | mr | 12118 | 0.369812 | 0.042 | 0.037 | 2.004 | PASS | fse=1 |
| fse | nci | 10000 | 0.305176 | 0.049 | 0.035 | 3.613 | PASS | fse=1 |
| fse | ooffice | 25182 | 0.768494 | 0.049 | 0.038 | 4.004 | PASS | fse=1 |
| fse | osdb | 27311 | 0.833466 | 0.044 | 0.036 | 2.004 | PASS | fse=1 |
| fse | reymont | 20364 | 0.621460 | 0.051 | 0.039 | 2.004 | PASS | fse=1 |
| fse | samba | 19959 | 0.609100 | 0.048 | 0.037 | 3.613 | PASS | fse=1 |
| fse | sao | 30507 | 0.931000 | 0.050 | 0.038 | 3.609 | PASS | fse=1 |
| fse | webster | 21014 | 0.641296 | 0.060 | 0.039 | 2.004 | PASS | fse=1 |
| fse | x-ray | 27018 | 0.824524 | 0.049 | 0.037 | 3.613 | PASS | fse=1 |
| fse | xml | 21653 | 0.660797 | 0.048 | 0.038 | 3.613 | PASS | fse=1 |
| rans | dickens | 19469 | 0.594147 | 0.045 | 0.048 | 3.492 | PASS | rans=1 |
| rans | mozilla | 21742 | 0.663513 | 0.049 | 0.038 | 3.629 | PASS | rans=1 |
| rans | mr | 12528 | 0.382324 | 0.044 | 0.038 | 2.004 | PASS | rans=1 |
| rans | nci | 10095 | 0.308075 | 0.050 | 0.037 | 3.617 | PASS | rans=1 |
| rans | ooffice | 25655 | 0.782928 | 0.039 | 0.038 | 2.004 | PASS | rans=1 |
| rans | osdb | 27825 | 0.849152 | 0.044 | 0.038 | 2.004 | PASS | rans=1 |
| rans | reymont | 20482 | 0.625061 | 0.046 | 0.038 | 3.238 | PASS | rans=1 |
| rans | samba | 20094 | 0.613220 | 0.050 | 0.036 | 3.625 | PASS | rans=1 |
| rans | sao | 30960 | 0.944824 | 0.038 | 0.036 | 2.000 | PASS | rans=1 |
| rans | webster | 21165 | 0.645905 | 0.061 | 0.039 | 2.617 | PASS | rans=1 |
| rans | x-ray | 27517 | 0.839752 | 0.052 | 0.038 | 4.004 | PASS | rans=1 |
| rans | xml | 21775 | 0.664520 | 0.050 | 0.038 | 3.984 | PASS | rans=1 |
| donor-match | dickens | 12951 | 0.395233 | 18.746 | 18.748 | 129.691 | PASS | donor-match=1 |
| donor-match | mozilla | 10890 | 0.332336 | 19.247 | 19.117 | 129.426 | PASS | donor-match=1 |
| donor-match | mr | 8474 | 0.258606 | 18.800 | 18.659 | 129.785 | PASS | donor-match=1 |
| donor-match | nci | 4879 | 0.148895 | 18.808 | 18.798 | 129.902 | PASS | donor-match=1 |
| donor-match | ooffice | 13659 | 0.416840 | 19.151 | 19.191 | 129.680 | PASS | donor-match=1 |
| donor-match | osdb | 15142 | 0.462097 | 19.250 | 19.356 | 129.887 | PASS | donor-match=1 |
| donor-match | reymont | 9361 | 0.285675 | 18.796 | 18.889 | 129.543 | PASS | donor-match=1 |
| donor-match | samba | 11173 | 0.340973 | 18.915 | 18.825 | 129.906 | PASS | donor-match=1 |
| donor-match | sao | 23756 | 0.724976 | 18.997 | 19.231 | 129.902 | PASS | donor-match=1 |
| donor-match | webster | 12539 | 0.382660 | 19.125 | 19.832 | 129.426 | PASS | donor-match=1 |
| donor-match | x-ray | 19389 | 0.591705 | 18.927 | 18.811 | 129.902 | PASS | donor-match=1 |
| donor-match | xml | 5016 | 0.153076 | 18.416 | 18.329 | 129.902 | PASS | donor-match=1 |
| bwt-zstd | dickens | 14061 | 0.429108 | 0.063 | 0.064 | 4.695 | PASS | bwt-zstd=1 |
| bwt-zstd | mozilla | 12081 | 0.368683 | 0.053 | 0.039 | 4.594 | PASS | bwt-zstd=1 |
| bwt-zstd | mr | 7934 | 0.242126 | 0.059 | 0.053 | 2.004 | PASS | bwt-zstd=1 |
| bwt-zstd | nci | 2775 | 0.084686 | 0.052 | 0.037 | 4.582 | PASS | bwt-zstd=1 |
| bwt-zstd | ooffice | 15371 | 0.469086 | 0.045 | 0.051 | 4.109 | PASS | bwt-zstd=1 |
| bwt-zstd | osdb | 17505 | 0.534210 | 0.049 | 0.054 | 4.574 | PASS | bwt-zstd=1 |
| bwt-zstd | reymont | 9556 | 0.291626 | 0.061 | 0.039 | 4.684 | PASS | bwt-zstd=1 |
| bwt-zstd | samba | 12113 | 0.369659 | 0.047 | 0.038 | 2.004 | PASS | bwt-zstd=1 |
| bwt-zstd | sao | 24720 | 0.754395 | 0.043 | 0.037 | 2.004 | PASS | bwt-zstd=1 |
| bwt-zstd | webster | 13874 | 0.423401 | 0.061 | 0.038 | 2.004 | PASS | bwt-zstd=1 |
| bwt-zstd | x-ray | 20054 | 0.612000 | 0.062 | 0.038 | 2.004 | PASS | bwt-zstd=1 |
| bwt-zstd | xml | 3024 | 0.092285 | 0.063 | 0.054 | 3.770 | PASS | bwt-zstd=1 |
| bwt-mtf-zstd | dickens | 13057 | 0.398468 | 0.057 | 0.047 | 3.336 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | mozilla | 11408 | 0.348145 | 0.053 | 0.052 | 4.582 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | mr | 7586 | 0.231506 | 0.058 | 0.038 | 4.656 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | nci | 2721 | 0.083038 | 0.064 | 0.038 | 2.004 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | ooffice | 14605 | 0.445709 | 0.056 | 0.038 | 4.621 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | osdb | 17562 | 0.535950 | 0.053 | 0.051 | 4.605 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | reymont | 8700 | 0.265503 | 0.080 | 0.038 | 4.672 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | samba | 11163 | 0.340668 | 0.049 | 0.053 | 2.750 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | sao | 25217 | 0.769562 | 0.062 | 0.053 | 4.664 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | webster | 12804 | 0.390747 | 0.049 | 0.051 | 4.105 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | x-ray | 19931 | 0.608246 | 0.064 | 0.038 | 4.695 | PASS | bwt-mtf-zstd=1 |
| bwt-mtf-zstd | xml | 2834 | 0.086487 | 0.062 | 0.039 | 4.656 | PASS | bwt-mtf-zstd=1 |
| bwt-rlt-zstd | dickens | 14522 | 0.443176 | 0.076 | 0.050 | 4.113 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | mozilla | 12319 | 0.375946 | 0.067 | 0.039 | 4.250 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | mr | 7918 | 0.241638 | 0.040 | 0.039 | 2.004 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | nci | 2923 | 0.089203 | 0.050 | 0.038 | 4.020 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | ooffice | 15676 | 0.478394 | 0.058 | 0.037 | 4.660 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | osdb | 17822 | 0.543884 | 0.039 | 0.037 | 2.004 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | reymont | 9857 | 0.300812 | 0.064 | 0.038 | 4.598 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | samba | 12578 | 0.383850 | 0.049 | 0.037 | 4.668 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | sao | 24882 | 0.759338 | 0.046 | 0.036 | 3.777 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | webster | 14319 | 0.436981 | 0.048 | 0.035 | 2.004 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | x-ray | 20283 | 0.618988 | 0.049 | 0.037 | 4.664 | PASS | bwt-rlt-zstd=1 |
| bwt-rlt-zstd | xml | 3043 | 0.092865 | 0.048 | 0.038 | 4.051 | PASS | bwt-rlt-zstd=1 |
| x86-bcj-zstd | dickens | 13216 | 0.403320 | 0.059 | 0.049 | 4.531 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | mozilla | 9561 | 0.291779 | 0.049 | 0.039 | 4.430 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | mr | 7962 | 0.242981 | 0.059 | 0.037 | 4.516 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | nci | 2480 | 0.075684 | 0.070 | 0.038 | 4.484 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | ooffice | 11362 | 0.346741 | 0.055 | 0.039 | 4.504 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | osdb | 15145 | 0.462189 | 0.058 | 0.038 | 4.492 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | reymont | 9108 | 0.277954 | 0.050 | 0.038 | 4.441 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | samba | 11027 | 0.336517 | 0.048 | 0.038 | 4.426 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | sao | 24423 | 0.745331 | 0.047 | 0.054 | 2.004 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | webster | 12772 | 0.389771 | 0.044 | 0.038 | 2.004 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | x-ray | 22464 | 0.685547 | 0.046 | 0.038 | 4.426 | PASS | x86-bcj-zstd=1 |
| x86-bcj-zstd | xml | 2713 | 0.082794 | 0.048 | 0.038 | 4.422 | PASS | x86-bcj-zstd=1 |
| shuffle-zstd | dickens | 17630 | 0.538025 | 0.058 | 0.045 | 4.445 | PASS | shuffle-zstd=1 |
| shuffle-zstd | mozilla | 11220 | 0.342407 | 0.053 | 0.036 | 4.527 | PASS | shuffle-zstd=1 |
| shuffle-zstd | mr | 7493 | 0.228668 | 0.058 | 0.054 | 4.574 | PASS | shuffle-zstd=1 |
| shuffle-zstd | nci | 2760 | 0.084229 | 0.087 | 0.038 | 4.527 | PASS | shuffle-zstd=1 |
| shuffle-zstd | ooffice | 15433 | 0.470978 | 0.058 | 0.038 | 4.543 | PASS | shuffle-zstd=1 |
| shuffle-zstd | osdb | 17796 | 0.543091 | 0.056 | 0.036 | 4.559 | PASS | shuffle-zstd=1 |
| shuffle-zstd | reymont | 13009 | 0.397003 | 0.062 | 0.038 | 4.562 | PASS | shuffle-zstd=1 |
| shuffle-zstd | samba | 14970 | 0.456848 | 0.055 | 0.050 | 4.441 | PASS | shuffle-zstd=1 |
| shuffle-zstd | sao | 27721 | 0.845978 | 0.051 | 0.048 | 2.355 | PASS | shuffle-zstd=1 |
| shuffle-zstd | webster | 16844 | 0.514038 | 0.046 | 0.037 | 4.414 | PASS | shuffle-zstd=1 |
| shuffle-zstd | x-ray | 17977 | 0.548615 | 0.064 | 0.039 | 4.574 | PASS | shuffle-zstd=1 |
| shuffle-zstd | xml | 3483 | 0.106293 | 0.065 | 0.038 | 4.516 | PASS | shuffle-zstd=1 |
| bitshuffle-zstd | dickens | 25969 | 0.792511 | 0.054 | 0.046 | 4.543 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | mozilla | 17365 | 0.529938 | 0.068 | 0.038 | 4.551 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | mr | 7954 | 0.242737 | 0.059 | 0.038 | 4.527 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | nci | 6587 | 0.201019 | 0.071 | 0.037 | 4.535 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | ooffice | 25593 | 0.781036 | 0.074 | 0.039 | 4.543 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | osdb | 29581 | 0.902740 | 0.059 | 0.038 | 2.004 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | reymont | 25429 | 0.776031 | 0.048 | 0.038 | 4.496 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | samba | 23428 | 0.714966 | 0.046 | 0.037 | 4.430 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | sao | 31336 | 0.956299 | 0.050 | 0.037 | 4.527 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | webster | 26475 | 0.807953 | 0.044 | 0.037 | 2.012 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | x-ray | 17975 | 0.548553 | 0.049 | 0.037 | 4.523 | PASS | bitshuffle-zstd=1 |
| bitshuffle-zstd | xml | 17645 | 0.538483 | 0.051 | 0.034 | 4.438 | PASS | bitshuffle-zstd=1 |
| delta-zstd | dickens | 15849 | 0.483673 | 0.071 | 0.046 | 4.543 | PASS | delta-zstd=1 |
| delta-zstd | mozilla | 12840 | 0.391846 | 0.068 | 0.038 | 4.543 | PASS | delta-zstd=1 |
| delta-zstd | mr | 8838 | 0.269714 | 0.089 | 0.037 | 4.559 | PASS | delta-zstd=1 |
| delta-zstd | nci | 3002 | 0.091614 | 0.101 | 0.038 | 4.539 | PASS | delta-zstd=1 |
| delta-zstd | ooffice | 16962 | 0.517639 | 0.056 | 0.038 | 4.527 | PASS | delta-zstd=1 |
| delta-zstd | osdb | 17324 | 0.528687 | 0.056 | 0.037 | 4.527 | PASS | delta-zstd=1 |
| delta-zstd | reymont | 11100 | 0.338745 | 0.062 | 0.037 | 4.535 | PASS | delta-zstd=1 |
| delta-zstd | samba | 13148 | 0.401245 | 0.062 | 0.039 | 4.539 | PASS | delta-zstd=1 |
| delta-zstd | sao | 27206 | 0.830261 | 0.050 | 0.038 | 4.504 | PASS | delta-zstd=1 |
| delta-zstd | webster | 15520 | 0.473633 | 0.058 | 0.038 | 4.539 | PASS | delta-zstd=1 |
| delta-zstd | x-ray | 20185 | 0.615997 | 0.048 | 0.037 | 4.445 | PASS | delta-zstd=1 |
| delta-zstd | xml | 3278 | 0.100037 | 0.081 | 0.037 | 4.508 | PASS | delta-zstd=1 |
| fastpfor | dickens | 31886 | 0.973083 | 0.047 | 0.048 | 2.004 | PASS | fastpfor=1 |
| fastpfor | mozilla | 25926 | 0.791199 | 0.049 | 0.039 | 3.863 | PASS | fastpfor=1 |
| fastpfor | mr | 20874 | 0.637024 | 0.045 | 0.038 | 2.004 | PASS | fastpfor=1 |
| fastpfor | nci | 31066 | 0.948059 | 0.043 | 0.037 | 2.004 | PASS | fastpfor=1 |
| fastpfor | ooffice | 29446 | 0.898621 | 0.041 | 0.038 | 2.004 | PASS | fastpfor=1 |
| fastpfor | osdb | 32478 | 0.991150 | 0.041 | 0.037 | 2.004 | PASS | fastpfor=1 |
| fastpfor | reymont | 31886 | 0.973083 | 0.059 | 0.038 | 2.004 | PASS | fastpfor=1 |
| fastpfor | samba | 29454 | 0.898865 | 0.049 | 0.038 | 3.836 | PASS | fastpfor=1 |
| fastpfor | sao | 32910 | 1.004333 | 0.047 | 0.037 | 3.355 | PASS | fastpfor=1 |
| fastpfor | webster | 31886 | 0.973083 | 0.045 | 0.037 | 3.316 | PASS | fastpfor=1 |
| fastpfor | x-ray | 28574 | 0.872009 | 0.047 | 0.038 | 3.371 | PASS | fastpfor=1 |
| fastpfor | xml | 31638 | 0.965515 | 0.046 | 0.037 | 3.836 | PASS | fastpfor=1 |
| bcj2-zstd | dickens | 13238 | 0.403992 | 0.054 | 0.047 | 5.121 | PASS | bcj2-zstd=1 |
| bcj2-zstd | mozilla | 9579 | 0.292328 | 0.049 | 0.038 | 5.102 | PASS | bcj2-zstd=1 |
| bcj2-zstd | mr | 7972 | 0.243286 | 0.044 | 0.053 | 3.648 | PASS | bcj2-zstd=1 |
| bcj2-zstd | nci | 2505 | 0.076447 | 0.066 | 0.037 | 5.262 | PASS | bcj2-zstd=1 |
| bcj2-zstd | ooffice | 12934 | 0.394714 | 0.046 | 0.038 | 2.059 | PASS | bcj2-zstd=1 |
| bcj2-zstd | osdb | 15154 | 0.462463 | 0.043 | 0.038 | 2.004 | PASS | bcj2-zstd=1 |
| bcj2-zstd | reymont | 9127 | 0.278534 | 0.049 | 0.038 | 5.086 | PASS | bcj2-zstd=1 |
| bcj2-zstd | samba | 11047 | 0.337128 | 0.048 | 0.038 | 5.102 | PASS | bcj2-zstd=1 |
| bcj2-zstd | sao | 24447 | 0.746063 | 0.046 | 0.037 | 5.141 | PASS | bcj2-zstd=1 |
| bcj2-zstd | webster | 12792 | 0.390381 | 0.045 | 0.037 | 2.004 | PASS | bcj2-zstd=1 |
| bcj2-zstd | x-ray | 22430 | 0.684509 | 0.043 | 0.038 | 2.004 | PASS | bcj2-zstd=1 |
| bcj2-zstd | xml | 2734 | 0.083435 | 0.055 | 0.048 | 5.070 | PASS | bcj2-zstd=1 |
| record-transpose-zstd | dickens | 19532 | 0.596069 | 0.053 | 0.048 | 4.430 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | mozilla | 14402 | 0.439514 | 0.055 | 0.048 | 4.438 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | mr | 9086 | 0.277283 | 0.061 | 0.054 | 4.551 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | nci | 4718 | 0.143982 | 0.074 | 0.038 | 4.523 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | ooffice | 20800 | 0.634766 | 0.058 | 0.038 | 4.539 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | osdb | 26493 | 0.808502 | 0.044 | 0.037 | 2.004 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | reymont | 18766 | 0.572693 | 0.047 | 0.038 | 3.418 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | samba | 18402 | 0.561584 | 0.049 | 0.037 | 4.434 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | sao | 29064 | 0.886963 | 0.046 | 0.038 | 4.449 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | webster | 21114 | 0.644348 | 0.045 | 0.038 | 2.004 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | x-ray | 18829 | 0.574615 | 0.050 | 0.037 | 4.480 | PASS | record-transpose-zstd=1 |
| record-transpose-zstd | xml | 8981 | 0.274078 | 0.050 | 0.036 | 4.430 | PASS | record-transpose-zstd=1 |
| jpegls | dickens | 30616 | 0.934326 | 0.049 | 0.063 | 2.215 | PASS | jpegls=1 |
| jpegls | mozilla | 24706 | 0.753967 | 0.055 | 0.053 | 3.715 | PASS | jpegls=1 |
| jpegls | mr | 12364 | 0.377319 | 0.044 | 0.038 | 2.004 | PASS | jpegls=1 |
| jpegls | nci | 18509 | 0.564850 | 0.058 | 0.053 | 2.004 | PASS | jpegls=1 |
| jpegls | ooffice | 30781 | 0.939362 | 0.059 | 0.037 | 2.004 | PASS | jpegls=1 |
| jpegls | osdb | 31715 | 0.967865 | 0.052 | 0.038 | 3.707 | PASS | jpegls=1 |
| jpegls | reymont | 28084 | 0.857056 | 0.052 | 0.046 | 2.004 | PASS | jpegls=1 |
| jpegls | samba | 27625 | 0.843048 | 0.051 | 0.048 | 3.223 | PASS | jpegls=1 |
| jpegls | sao | 34802 | 1.062073 | 0.048 | 0.037 | 3.098 | PASS | jpegls=1 |
| jpegls | webster | 30745 | 0.938263 | 0.051 | 0.062 | 2.012 | PASS | jpegls=1 |
| jpegls | x-ray | 27154 | 0.828674 | 0.049 | 0.038 | 3.348 | PASS | jpegls=1 |
| jpegls | xml | 25671 | 0.783417 | 0.047 | 0.038 | 2.012 | PASS | jpegls=1 |
| flac-residual | dickens | 31542 | 0.962585 | 0.061 | 0.046 | 4.293 | PASS | flac-residual=1 |
| flac-residual | mozilla | 31925 | 0.974274 | 0.067 | 0.052 | 4.531 | PASS | flac-residual=1 |
| flac-residual | mr | 26374 | 0.804871 | 0.062 | 0.053 | 4.512 | PASS | flac-residual=1 |
| flac-residual | nci | 27308 | 0.833374 | 0.055 | 0.038 | 4.297 | PASS | flac-residual=1 |
| flac-residual | ooffice | 33375 | 1.018524 | 0.061 | 0.067 | 4.316 | PASS | flac-residual=1 |
| flac-residual | osdb | 31654 | 0.966003 | 0.064 | 0.038 | 2.004 | PASS | flac-residual=1 |
| flac-residual | reymont | 30798 | 0.939880 | 0.061 | 0.038 | 4.465 | PASS | flac-residual=1 |
| flac-residual | samba | 31047 | 0.947479 | 0.064 | 0.037 | 4.535 | PASS | flac-residual=1 |
| flac-residual | sao | 33309 | 1.016510 | 0.062 | 0.054 | 4.488 | PASS | flac-residual=1 |
| flac-residual | webster | 31466 | 0.960266 | 0.060 | 0.039 | 4.340 | PASS | flac-residual=1 |
| flac-residual | x-ray | 17930 | 0.547180 | 0.062 | 0.054 | 4.445 | PASS | flac-residual=1 |
| flac-residual | xml | 30013 | 0.915924 | 0.063 | 0.070 | 2.297 | PASS | flac-residual=1 |
| brotli-text | dickens | 11322 | 0.345520 | 0.086 | 0.043 | 6.066 | PASS | brotli-text=1 |
| brotli-text | mozilla | 8869 | 0.270660 | 0.080 | 0.037 | 5.984 | PASS | brotli-text=1 |
| brotli-text | mr | 6938 | 0.211731 | 0.075 | 0.037 | 6.148 | PASS | brotli-text=1 |
| brotli-text | nci | 2366 | 0.072205 | 0.097 | 0.040 | 6.488 | PASS | brotli-text=1 |
| brotli-text | ooffice | 12584 | 0.384033 | 0.072 | 0.053 | 5.812 | PASS | brotli-text=1 |
| brotli-text | osdb | 14231 | 0.434296 | 0.080 | 0.039 | 5.820 | PASS | brotli-text=1 |
| brotli-text | reymont | 8742 | 0.266785 | 0.077 | 0.038 | 6.082 | PASS | brotli-text=1 |
| brotli-text | samba | 9204 | 0.280884 | 0.078 | 0.053 | 6.051 | PASS | brotli-text=1 |
| brotli-text | sao | 23113 | 0.705353 | 0.097 | 0.052 | 5.797 | PASS | brotli-text=1 |
| brotli-text | webster | 10905 | 0.332794 | 0.077 | 0.051 | 5.973 | PASS | brotli-text=1 |
| brotli-text | x-ray | 18319 | 0.559052 | 0.097 | 0.038 | 5.688 | PASS | brotli-text=1 |
| brotli-text | xml | 2381 | 0.072662 | 0.076 | 0.038 | 6.250 | PASS | brotli-text=1 |
| cmix-word-zstd | dickens | 10890 | 0.332336 | 0.075 | 0.064 | 7.828 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | mozilla | 10622 | 0.324158 | 0.068 | 0.053 | 7.746 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | mr | 8264 | 0.252197 | 0.075 | 0.055 | 7.746 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | nci | 2521 | 0.076935 | 0.085 | 0.052 | 7.746 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | ooffice | 14885 | 0.454254 | 0.071 | 0.053 | 7.762 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | osdb | 16981 | 0.518219 | 0.072 | 0.053 | 6.191 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | reymont | 9258 | 0.282532 | 0.064 | 0.051 | 7.742 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | samba | 9020 | 0.275269 | 0.064 | 0.070 | 7.727 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | sao | 28778 | 0.878235 | 0.065 | 0.052 | 7.762 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | webster | 11105 | 0.338898 | 0.076 | 0.068 | 7.832 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | x-ray | 25749 | 0.785797 | 0.065 | 0.052 | 4.664 | PASS | cmix-word-zstd=1 |
| cmix-word-zstd | xml | 2499 | 0.076263 | 0.080 | 0.055 | 7.855 | PASS | cmix-word-zstd=1 |
| neural-lstm | dickens | 14609 | 0.445831 | 18.048 | 18.005 | 120.816 | PASS | neural-lstm=1 |
| neural-lstm | mozilla | 11193 | 0.341583 | 18.529 | 18.541 | 120.816 | PASS | neural-lstm=1 |
| neural-lstm | mr | 6973 | 0.212799 | 18.295 | 18.148 | 120.801 | PASS | neural-lstm=1 |
| neural-lstm | nci | 4533 | 0.138336 | 18.212 | 18.088 | 120.801 | PASS | neural-lstm=1 |
| neural-lstm | ooffice | 13966 | 0.426208 | 18.629 | 18.661 | 120.816 | PASS | neural-lstm=1 |
| neural-lstm | osdb | 22398 | 0.683533 | 18.642 | 18.496 | 120.797 | PASS | neural-lstm=1 |
| neural-lstm | reymont | 10647 | 0.324921 | 18.090 | 17.982 | 120.816 | PASS | neural-lstm=1 |
| neural-lstm | samba | 14103 | 0.430389 | 18.017 | 17.989 | 120.812 | PASS | neural-lstm=1 |
| neural-lstm | sao | 23817 | 0.726837 | 18.354 | 18.015 | 120.801 | PASS | neural-lstm=1 |
| neural-lstm | webster | 14627 | 0.446381 | 18.173 | 17.983 | 120.816 | PASS | neural-lstm=1 |
| neural-lstm | x-ray | 18618 | 0.568176 | 18.070 | 18.200 | 120.812 | PASS | neural-lstm=1 |
| neural-lstm | xml | 6544 | 0.199707 | 17.845 | 17.864 | 120.801 | PASS | neural-lstm=1 |
| shared-neural-lstm | dickens | 14777 | 0.450958 | 18.198 | 17.993 | 120.828 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | mozilla | 11241 | 0.343048 | 18.652 | 18.593 | 120.828 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | mr | 7009 | 0.213898 | 18.178 | 18.292 | 120.812 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | nci | 4571 | 0.139496 | 18.152 | 18.152 | 120.812 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | ooffice | 13924 | 0.424927 | 18.727 | 18.589 | 120.828 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | osdb | 22187 | 0.677094 | 18.576 | 18.595 | 120.812 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | reymont | 10568 | 0.322510 | 18.028 | 18.006 | 120.828 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | samba | 13752 | 0.419678 | 18.354 | 18.281 | 120.812 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | sao | 23810 | 0.726624 | 18.225 | 18.067 | 120.812 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | webster | 14722 | 0.449280 | 18.074 | 18.063 | 120.828 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | x-ray | 18194 | 0.555237 | 18.300 | 18.218 | 120.812 | PASS | shared-neural-lstm=1 |
| shared-neural-lstm | xml | 6286 | 0.191833 | 17.953 | 17.998 | 120.812 | PASS | shared-neural-lstm=1 |
