# HybridZip R2 F3 Fast Block Executor Experiment Design

## Decision

F3 implements independent block execution only for `ENC_FAST_V1`
(`--r2-mode=fast`). It preserves the single HZ02 format, HZ01 decoding, and
the numeric meaning of HZ02 IDs `0..43`. Thread count is encoder-local state:
it is never serialized into an archive.

`auto`, `auto-k2`, `auto-k4`, `auto-k8`, and every forced policy remain
single-threaded in F3. Full Auto's `BlockPlanner` maintains family telemetry,
so parallel execution would not be a transparent implementation change.

## Implementation Target

`FastBlockExecutor` receives bounded, indexed raw blocks. Each worker owns a
separate Fast `BlockPlanner`, which evaluates the four existing top-level
candidates: stored, raw Mode-43 zstd, best transformed Mode-43 zstd, and LZ4.
Completed results may arrive in arbitrary order. The caller takes result `N`
before serializing result `N + 1`, so each HZ02 block header, CRC, extension
metadata, and payload remain in input order.

The queue limit is `2 * worker_count` blocks. This bounds uncompressed work
memory independently of input length. zstd itself remains single-threaded;
HybridZip supplies the only block-level concurrency.

## Current Gate: F3.2

| Item | Fixed value |
| --- | --- |
| Input | deterministic 1,024-byte raw file |
| Internal block size | 256 bytes (four HZ02 blocks) |
| Policy | `fast` / Fast K=4 |
| Runs | one worker, then two workers |
| Required archive evidence | complete HZ02 byte length and SHA-256 |
| Required decode evidence | decoded SHA-256 equal to input SHA-256 |
| Determinism gate | one- and two-worker archive SHA-256 are equal |
| Compatibility gate | no HZ01/HZ02 format change; Mode-43 remains append-only |

This gate deliberately does not measure throughput, latency, RAM, router
regret, PAQ8px ratio, Silesia, Tencent/OASum, or GPU performance.

## Follow-on Matrix: F3.4 / E6-Fast-K4

After F3.2 passes, create a new non-overwriting Fast package on the frozen
36-row leading-prefix Silesia manifest. For each 32/64/128 KiB input scope and
internal block size, run one warmup plus three retained repeats at worker
counts one and a fixed CPU-pool value. The matrix runner exposes this as
`-FastThreadCount`; it passes the same value to every Fast child as
`--threads` and rejects it for E5. Record complete archive bytes, bpb,
decoded hashes, executable hash, wall/CPU time, encode/decode MB/s, P50/P95
queue-plus-service and service-only block latency, process RAM, worker count,
and candidate telemetry.

The F3.4 acceptance floor remains >= 0.16 MB/s for both encode and decode in
every measured cell. It is a Fast-policy result only and cannot support the
PAQ8px ratio target.

## Rejection Conditions

- A result is serialized out of input order.
- Any 1 KiB decoded byte differs from the source.
- Worker count changes the complete archive bytes.
- A non-Fast policy is silently parallelized.
- A timing claim is made from the F3.2 smoke gate.
