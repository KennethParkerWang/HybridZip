# Statistical Appendix

## Design

- Paired unit: Silesia file (12 units), each evaluated at a 32 KiB leading prefix.
- Repeats/seeds: one per mode and case; no repeated-run variance estimate.
- Metric: complete archive bpb; lower is better.
- Inferential tests: none; independence, normality, and variance assumptions are not used.
- Multiple-comparison correction: not applicable because no hypothesis tests are reported.

## Descriptive Summary

| Quantity | n | Mean | SD | Min | Median | Max |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Auto bpb across files | 12 | 2.028809 | 1.232130 | 0.327393 | 1.959595 | 4.591064 |
| Oracle bpb across files | 12 | 2.028809 | 1.232130 | 0.327393 | 1.959595 | 4.591064 |
| Auto-oracle gap (bytes) | 12 | 0.000000 | 0.000000 | 0 | 0.000000 | 0 |

## Exact Counts

- Validated rows: 528/528.
- Complete packages: 44/44.
- Byte-exact roundtrips: 528/528.
- Auto gap-positive cases: 0/12.
- Forced oracle wins, ties counted: 12; generic SSE 7, detected SSE 5.

## Interpretation Boundary

The SD values describe variation across the 12 file types, not run-to-run uncertainty. No significance test or population-level ranking is valid from this single-prefix, single-run design. The result is an engineering selection observation and must be repeated at additional sizes and domains.
