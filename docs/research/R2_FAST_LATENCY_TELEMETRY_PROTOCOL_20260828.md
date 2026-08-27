# R2 Fast Block Latency Telemetry Protocol

## Scope

This protocol implements the uploaded R2 decision's required Fast-policy
block-latency fields without changing HZ01, HZ02 archive bytes, or the
decoder-visible mode registry. It applies only when `--r2-mode=fast` uses the
bounded `FastBlockExecutor`.

## Measurements

For each Fast block, the encoder records two `steady_clock` durations in
nanoseconds:

| Field | Start | End | Excludes |
| --- | --- | --- | --- |
| queue-plus-service | immediately before bounded-queue insertion | after `BlockPlanner::plan` returns | input reading and canonical archive serialization |
| service-only | immediately before `BlockPlanner::plan` | after `BlockPlanner::plan` returns | queue waiting, input reading, and archive serialization |

The CLI emits each raw sample list plus nearest-rank P50/P95. The E6 matrix
persists raw lists in `matrix_rows.csv`, combines all retained block samples
in a summary cell, and calculates exact nearest-rank P50/P95. Warmup rows are
kept in the matrix but excluded from the E6 summaries. The parser rejects a
row when its reported P50/P95 does not reproduce from its raw samples, when
the paired sample counts differ, or when queue-plus-service is below
service-only.

## Target and gates

| Target | Required evidence | Pass condition |
| --- | --- | --- |
| L1 correctness | 1 KiB deterministic input, 4 x 256-byte blocks, 1 and 2 workers | byte-exact decode; identical archive SHA-256 across worker counts; four nonempty paired latency samples per run |
| E6 throughput | frozen Silesia inputs at 32/64/128 KiB, one warmup plus three retained repeats | byte-exact decode and encode/decode median aggregate throughput >= 0.16 MB/s in every block-size/scope cell |
| E6 latency | same retained rows | report exact block queue-plus-service and service-only P50/P95, plus wall-clock encode/decode timing separately |

No latency result from the 1 KiB gate is a throughput or corpus performance
claim. The complete post-change E6 matrix remains separately authorized.
