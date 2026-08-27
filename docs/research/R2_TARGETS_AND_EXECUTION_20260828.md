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

- Baseline source milestone: `439e948`.
- Current E5 forced-oracle evidence-binding milestone: `996d59d`.
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
| E5 | K=2/K=4/K=8 versus full Auto | Report complete-byte regret, selected-mode coverage, and a matching 32 KiB forced-mode tie-aware oracle | Forced-ledger/E5 evidence binding and self-test complete; runtime queued |
| E6 | Fast policy, warmup plus 3 retained repeats | Every 32/64/128 KiB input/block cell byte-exact; encode/decode each >= 0.16 MB/s | Passed for current Fast baseline |
| F1 | 28-feature fixed-point ranker | Feature/model implementation is deterministic; one model identity per matrix package; no-leakage labels and measured router budget remain required | Bootstrap identity telemetry implemented; not promoted |
| F2 | `MODE_FAST_EXT_V1` | Pinned donor, independent standard-frame decode, old archives decode unchanged | 1 KiB gate passed; corpus rerun pending |
| F3 | Block executor | Canonical archive order and byte-exact repeats; measure before/after Fast throughput | Fast-only executor/1 KiB gate passed; guarded worker-count matrix preflight passed; post-change timing pending |
| F4 | GPU `LZ_RANS_V1` | CPU reference decoder; end-to-end >= 8 MB/s at every required size | Blocked by F2/F3 |

## Runtime Protocol

Every accepted row records input identity, executable hash, command, archive
bytes/hash, decoded hash, byte-exact outcome, block size, candidate telemetry,
fixed-point ranker version/CRC32/SHA-256, wall time, and sampled RAM. A package
is rejected if those ranker fields identify more than one model. `bpb = 8 *
archive_bytes / input_bytes`. Archive bytes include HZ02 headers, CRC,
metadata, transforms, and payload.

E3 command and E6 command write unique, non-overwriting packages under
`results/experiments/`. E6 runs first on an otherwise idle CPU; E3 starts only
after it completes because PAQ CPU load would invalidate Fast timing. E5 is
not co-scheduled because its full-Auto reference repeatedly evaluates PAQ
candidates and would likewise invalidate Fast timing.

`tools/run_r2_e5_e6_matrix.ps1 -Resume` supports an interrupted package only
when its stage, executable hash, dataset path, files, scopes, block sizes,
policies, repeats, and optional forced-ledger path exactly match the request.
When `-ForcedOracleLedgerPath` is given to E5, the runner validates the
completed 32 KiB forced ledger before any PAQ-heavy work, then writes tied
winner recall evidence to `<e5-package>\forced-oracle`. A complete package is
validated without re-running codecs, including its single ranker-model
identity and, when requested, the forced-oracle evidence. This makes a
multi-hour E5 run recoverable without weakening its evidence identity.

## Known Gaps

`BlockFeaturesV1` now exposes the specified 28 integer features and K=8 uses
a versioned 2,644-byte fixed-point bootstrap model with hard family gates.
It has not been fit from a forced-mode matrix, so no-leakage labels, router
budget timing, and held-out regret remain F1 acceptance requirements. Fast K=4
and the append-only Mode-43 extension are implemented, but their corpus-level
throughput matrix is pending. Fast now has a bounded independent-block thread
pool; Auto, shortlists, and forced modes remain serial. There is no GPU
backend.

The forced-mode ledger runner now pins a requested internal block size and the
new derivation tool rejects mixed executable/input/block evidence before
calculating tied winners. It is designed for 32 KiB one-block labels only;
the ledger and E5 runtime packages remain pending.

`third_party/zstd` identifies as 1.6.0. The decision requests zstd 1.5.7,
whose stated source archive is 2,434,947 bytes under BSD-3-Clause. No 1.5.7
download/import has occurred; present E6 results must be treated only as
current-build baseline measurements.

OASum is not downloaded. Its final `test.jsonl` is 1,065,019,104 bytes and
CC-BY-SA-3.0; an owner decision is required before materialization or a
Tencent-coverage claim.
