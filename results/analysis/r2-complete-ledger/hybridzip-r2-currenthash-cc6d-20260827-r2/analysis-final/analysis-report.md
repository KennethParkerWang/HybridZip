# HybridZip R2 Current-Hash Strict Analysis

## Analysis Question

Does the decoder-visible HZ02 Auto route match the minimum complete archive-byte
choice among all 43 forced modes on the declared Silesia prefix matrix, and
which candidates contribute to that result?

## Evidence Boundary

- Ledger: `hybridzip-r2-currenthash-cc6d-20260827-r2`
- Unit of analysis: one Silesia file, one 32 KiB leading prefix, one run per mode
- Cases: 12 files; 44 modes (Auto plus 43 forced); 528 validated rows
- Primary metric: complete `.hz2` archive bytes and `bpb = archive_bytes * 8 / input_bytes`; lower is better
- Integrity: every row has status `COMPLETE`, roundtrip `PASS`, and byte-exact input/archive/decoded SHA-256 checks
- Codec hash: `CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`

## Findings

Auto totals 99,720 bytes (2.028809 bpb) over
393,216 input bytes. The forced oracle has the same total, so the aggregate
gap is 0 bytes (2.028809 bpb). The per-case gap is zero for all
12 files. Auto selected `paq8px-detected-sse` for 5 cases and
`paq8px-generic-sse` for 7 cases; these are also the only forced modes that
won an archive-byte oracle case.

The ten lowest weighted forced rates are:

| Rank | Mode | Archive bytes | bpb | Oracle wins | Auto selections |
| ---: | --- | ---: | ---: | ---: | ---: |
| 1 | `paq8px-detected-sse` | 99,920 | 2.032878 | 5 | 5 |
| 2 | `paq8px-generic-sse` | 100,011 | 2.034729 | 7 | 7 |
| 3 | `predictive` | 127,356 | 2.591064 | 0 | 0 |
| 4 | `brotli-text` | 128,974 | 2.623983 | 0 | 0 |
| 5 | `ppmd8` | 129,382 | 2.632284 | 0 | 0 |
| 6 | `ppmd7` | 129,614 | 2.637004 | 0 | 0 |
| 7 | `paq8px-apm` | 132,062 | 2.686808 | 0 | 0 |
| 8 | `lzma` | 133,105 | 2.708028 | 0 | 0 |
| 9 | `x86-bcj-zstd` | 142,233 | 2.893738 | 0 | 0 |
| 10 | `zpaq` | 142,561 | 2.900411 | 0 | 0 |

## Claim Candidates

- Claim: On this exact 12-case, 32 KiB prefix matrix, Auto reached the complete forced-mode archive-byte oracle.
  - Source evidence: `per_case_oracle.tsv`, 12/12 zero-gap rows; `mode_aggregate.tsv`, Auto and oracle totals both 99,720 bytes.
  - Allowed wording: “matched on the evaluated matrix.”
  - Forbidden stronger wording: “globally optimal,” “best on Silesia,” or “generalizes to all files.”
  - Uncertainty: one prefix size and one run per case; no unseen-file evaluation.
  - Next check: repeat at 64/128 KiB and add independent file classes.
  - Decision: keep
- Claim: The two PAQ8px SSE candidates carried all observed oracle wins in this matrix.
  - Source evidence: 7 generic-SSE and 5 detected-SSE oracle-win rows; all other modes have zero.
  - Allowed wording: “the observed winners in this matrix.”
  - Forbidden stronger wording: “other donors are redundant for all workloads.”
  - Uncertainty: candidate coverage is narrow and the current routing cost is high.
  - Next check: evaluate heterogeneous segments and non-Silesia inputs.
  - Decision: keep

## Statistical Scope

This is descriptive, paired evidence with n=12 file cases and one run/seed per
case. It supports no p-value, confidence interval for repeated-run variation,
effect-size claim, or independence assumption. The exact Auto-oracle gap is
0 bytes in every case; the across-file Auto bpb mean is
2.028809 (SD 1.232130) and the oracle mean is
2.028809 (SD 1.232130).

## Limitations

- The 32 KiB leading prefixes are not a full Silesia benchmark.
- Timing and peak memory are engineering observations, not statistical estimates.
- Candidate-not-oracle-winner is a corpus-local signal; no donor source is deleted.
- Segment-level heterogeneity remains unmeasured in this authorized ledger.
