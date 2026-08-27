# HybridZip R2 Targets And Execution

## Decision

The current program keeps one HZ02 archive/decoder contract and evaluates two
encoder policies independently:

| Policy | Current form | Acceptance claim it may support |
| --- | --- | --- |
| `ENC_RATIO_V1` | Full Auto reference and deterministic `auto-k8` shortlist | Same-input complete archive bytes versus PAQ8px v216 `-1` |
| `ENC_FAST_V1` | `fast`, four candidates: stored, Mode-43 zstd extension/transforms, and LZ4 | CPU encode and decode throughput |
| `ENC_ORACLE` | Existing full `auto` portfolio | Router reference only; not a product policy |

These policies are not separate archive formats. HZ01 and all existing HZ02
IDs `0..42` remain decoder compatibility requirements. A result from one
policy cannot be used to claim the other policy's target.

## Current Baseline

- Committed source milestone: `439e948`.
- Frozen Tier-A inputs: 12 Silesia files with leading 32/64/128 KiB prefixes,
  `bench/manifests/silesia-leading-32-64-128.tsv` (36 rows; SHA-256
  `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D`).
- Existing 32 KiB R2 ledger: 528/528 byte-exact rows; full Auto 2.028809 bpb;
  full-Auto and forced-mode oracle are equal on that historical 12-case matrix.
- Implemented encoder policies: `auto`, `auto-k2`, `auto-k4`, `auto-k8`, and
  Fast K=4. K=2/K=4/K=8 ratio shortlists remain ablations; K=8 is not
  promoted.

## Measurable Objectives

| ID | Measurement | Gate | Status |
| --- | --- | --- | --- |
| E3 | PAQ8px v216 `-1` on all 36 frozen inputs | Every row has matching input/decoded SHA-256 and complete archive bytes | Passed |
| E5 | K=2/K=4/K=8 versus full Auto | Report complete-byte regret and selected-mode coverage; promotion additionally needs a matching forced-mode tie-aware oracle | Queued |
| E6 | Fast policy, warmup plus 3 retained repeats | Every 32/64/128 KiB input/block cell byte-exact; encode/decode each >= 0.16 MB/s | Passed for current Fast baseline |
| F1 | 28-feature fixed-point ranker | Deterministic across named builds; measured router budget; no-leakage labels | Queued |
| F2 | `MODE_FAST_EXT_V1` | Pinned donor, independent standard-frame decode, old archives decode unchanged | 1 KiB gate passed; corpus rerun pending |
| F3 | Block executor | Canonical archive order and byte-exact repeats; measure before/after Fast throughput | Queued |
| F4 | GPU `LZ_RANS_V1` | CPU reference decoder; end-to-end >= 8 MB/s at every required size | Blocked by F2/F3 |

## Runtime Protocol

Every accepted row records input identity, executable hash, command, archive
bytes/hash, decoded hash, byte-exact outcome, block size, candidate telemetry,
wall time, and sampled RAM. `bpb = 8 * archive_bytes / input_bytes`. Archive
bytes include HZ02 headers, CRC, metadata, transforms, and payload.

E3 command and E6 command write unique, non-overwriting packages under
`results/experiments/`. E6 runs first on an otherwise idle CPU; E3 starts only
after it completes because PAQ CPU load would invalidate Fast timing. E5 is
not co-scheduled because its full-Auto reference repeatedly evaluates PAQ
candidates and would likewise invalidate Fast timing.

`tools/run_r2_e5_e6_matrix.ps1 -Resume` supports an interrupted package only
when its stage, executable hash, dataset path, files, scopes, block sizes,
policies, and repeats exactly match the request. A complete package is
validated without re-running codecs. This makes a multi-hour E5 run
recoverable without weakening its evidence identity.

## Known Gaps

The current `BlockFeaturesV1` has a small rule-only feature set, not the
decision's 28-feature ranker. Fast K=4 and the append-only Mode-43 extension
are implemented, but their corpus-level throughput matrix is pending. There
is no independent-block thread pool or GPU backend.

`third_party/zstd` identifies as 1.6.0. The decision requests zstd 1.5.7,
whose stated source archive is 2,434,947 bytes under BSD-3-Clause. No 1.5.7
download/import has occurred; present E6 results must be treated only as
current-build baseline measurements.

OASum is not downloaded. Its final `test.jsonl` is 1,065,019,104 bytes and
CC-BY-SA-3.0; an owner decision is required before materialization or a
Tencent-coverage claim.
