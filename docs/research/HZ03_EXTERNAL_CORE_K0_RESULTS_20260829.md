# HZ03 External-Core Kill Test: K0 Results (2026-08-29)

## Scope

K0 is the staged 1 KiB byte-exact smoke test defined by
`HZ03_EXTERNAL_CORE_KILL_TEST_PROTOCOL_20260829.md`. It checks that each
candidate CLI can encode and decode the same input before any 32/64/128 KiB
comparison is started.

- Experiment: `hybridzip-external-killtest-k0-20260829`
- Input size: `1024` bytes
- Input SHA-256: `E051D1007607DE494C073DA3C29903D6C0ABFEE7A4C0609F560A340A1947B470`
- Result package: `results/experiments/hybridzip-external-killtest-k0-20260829-retry5/hybridzip-external-killtest-k0-20260829/`
- Source/format boundary: no HybridZip source, HZ01/HZ02 decoder, or archive-format change

## Results

All 11 candidates completed encoding and decoding with exit code `0`. Every
decoded output is `1024` bytes and has the exact input SHA-256. No timeout,
memory-limit, or failed row was recorded.

| Candidate | Archive bytes | Encode s | Decode s | Peak RAM MiB | Roundtrip |
| --- | ---: | ---: | ---: | ---: | --- |
| kanzi-l7 | 254 | 0.073253 | 0.040983 | 2.027 | PASS |
| kanzi-l8 | 299 | 0.043514 | 0.041333 | 2.027 | PASS |
| kanzi-l9 | 303 | 0.045759 | 0.041083 | 2.027 | PASS |
| libbsc-e2 | 1070 | 0.041997 | 0.042782 | 2.027 | PASS |
| paq8px-l1 | 31 | 0.386412 | 0.413707 | 537.188 | PASS |
| paq8px-l2 | 31 | 0.414834 | 0.428509 | 547.805 | PASS |
| paq8px-l3 | 31 | 0.398815 | 0.410346 | 566.449 | PASS |
| paq8px-l4 | 31 | 0.386087 | 0.424983 | 598.230 | PASS |
| xz-9e | 316 | 0.042322 | 0.041570 | 2.016 | PASS |
| hybridzip-auto | 80 | 3.303412 | 0.040653 | 535.262 | PASS |
| hybridzip-fast | 165 | 0.040814 | 0.042380 | 2.023 | PASS |

## Interpretation boundary

K0 proves only CLI availability and byte-exact roundtrip on a 1 KiB input. The
archive sizes are overhead-dominated and are not compression-ratio evidence.
It does not select a codec, establish full-file performance, or justify a new
HybridZip mode. The next authorized experiment is the same-input 32/64/128 KiB
comparison, staged from this unchanged candidate set.

## Reproduction

The runner is `tools/run_external_core_killtest.ps1`. The complete machine
readable evidence remains in `results.csv` and `experiment.json` in the result
package above; those generated archives are intentionally not committed.
