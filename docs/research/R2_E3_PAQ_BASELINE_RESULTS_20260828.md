# HybridZip R2 E3 Same-Input PAQ8px Baseline

## Result

The frozen leading-prefix PAQ8px v216 `-1` baseline is complete and
byte-exact. It supplies the first full same-input PAQ reference for all 12
Silesia files at leading 32, 64, and 128 KiB prefixes.

## Package And Verification

- Package: `results/experiments/paq8px-v216-level1-silesia-leading-e3-20260828/`
- PAQ executable SHA-256:
  `F79343702F596A4FA6C7CC3E25F2FA9C05199EAF11F06655B065F687E5F42533`.
- Input matrix: `bench/manifests/silesia-leading-32-64-128.tsv`.
- Rows: 36 of 36 `COMPLETE/PASS`; encode and decode exit code zero for every
  row.
- The result rows have 36 distinct `(file, scope)` keys and exactly match the
  manifest rows. Every input/decoded length and SHA-256 equals the frozen
  prefix identity.
- The E3 Dickens 32 KiB archive exactly reproduces the E2 smoke: 9,502 bytes,
  archive SHA-256 `01B6307B23E8B99B10B27D8DFCB76DADB4A143FA195490D350BA5BDCDCD77592`.

## Complete Archive Bytes

| Prefix scope | Input bytes | PAQ archive bytes | Archive fraction | bpb |
| --- | ---: | ---: | ---: | ---: |
| 32 KiB | 393,216 | 97,555 | 0.248095 | 1.984762 |
| 64 KiB | 786,432 | 182,710 | 0.232328 | 1.858622 |
| 128 KiB | 1,572,864 | 342,298 | 0.217627 | 1.741018 |
| All scopes | 2,752,512 | 622,563 | 0.226183 | 1.809440 |

PAQ encode/decode totals are 323.025/323.464 seconds. Maximum sampled peak
RAM is 1,147.49 MiB. These are single serial observations, not repeated timing
statistics.

## Ratio Boundary

The older 32 KiB HybridZip current-hash Auto ledger used the same 12 leading
prefixes and reported 99,720 archive bytes (2.028809 bpb). The new PAQ E3
reference reports 97,555 bytes (1.984762 bpb), a PAQ advantage of 2,165 bytes
or 2.1711% relative to the HybridZip archive total.

This establishes a current 32 KiB ratio gap in the historical Auto ledger. It
does not evaluate the newer working-tree executable, does not cover HybridZip
at 64/128 KiB, and does not authorize any claim of PAQ superiority or
inferiority. E5 remains necessary for current full-Auto/K=2/K=4/K=8 archive
measurements, and a matching forced-mode oracle remains necessary for
tie-aware shortlist promotion.
