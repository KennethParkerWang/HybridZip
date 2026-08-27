# HybridZip R2 Evidence-Gated Execution Plan

## Purpose

This plan applies the 2026-08-28 external research decision to the actual
HybridZip checkout. It begins with fair inputs and current fast paths, then
permits router and performance changes only when their archive-byte and
byte-exact evidence pass the stated gates.

## Baseline

- Git baseline: `baseline-r2-20260828` at `e8a0167`.
- Active Release SHA-256:
  `CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`.
- Current R2 ledger: 44 packages, 528/528 byte-exact passes, 12 leading-32-KiB
  Silesia prefixes, Auto 2.028809 bpb, and a 0-byte Auto/oracle gap on that
  matrix.

These results are not a fair PAQ8px comparison because the existing PAQ8px
suite uses centred slices. No target claim starts from this baseline.

## Objectives and Gates

| Target | Work | Acceptance gate |
| --- | --- | --- |
| R0 | Freeze leading Silesia 32/64/128 KiB inputs | 36 manifest rows; source and prefix SHA-256 match files |
| R1 | Run PAQ8px v216 `-1` on R0 inputs | Complete archive bytes and decoded SHA-256 match each input |
| R2 | Implement a K=8 ratio shortlist | Tie-aware winner recall >=99.5%, aggregate regret <=0.02% of full oracle bytes, both observed PAQ8px-SSE modes retained |
| R3 | Establish fast policy from existing zstd/transform modes | Byte-exact at 32/64/128 KiB; report P50/P95, RAM, and MB/s; CPU target is >=0.16 MB/s encode and decode |
| R4 | Add Tencent acceptance corpus | Requires prior approval of OASum download and CC-BY-SA-3.0 treatment |
| R5 | GPU LZ+rANS research | Only after a CPU reference fast path and exact CPU/GPU decode vectors exist |

## Implementation Order

1. Generate `bench/manifests/silesia-leading-32-64-128.tsv` from the canonical
   local corpus `F:\paq8px\silesia`.
2. Add a non-overwriting PAQ8px v216 runner that accepts this manifest and
   refuses codec execution unless explicitly authorized.
3. Run one manifest case at 32 KiB after its planned command, binary hash, and
   output path are visible. Verify input/archive/decoded SHA-256.
4. Only then authorize the remaining same-input PAQ8px matrix.
5. Use the already vendored zstd (`third_party/zstd`) and current HZ02 zstd /
   transform modes for the fast-policy design. Do not duplicate zstd under a
   new mode before existing modes are measured.
6. Add `BlockFeaturesV1`, then a rule-only K=8 shortlist. Preserve the current
   full Auto route as the oracle implementation.

## Execution Status

- **E1 complete, no codec runtime:**
  `bench/manifests/silesia-leading-32-64-128.tsv` has the canonical 36 leading
  prefixes and a file SHA-256 of
  `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D`.
- `tools/generate_silesia_prefix_manifest.ps1` refuses to replace a frozen
  manifest. `tools/run_paq8px_manifest_experiment.ps1` validates that exact
  matrix, defaults to `-ListOnly`, and requires
  `-AuthorizeRuntimeExperiment` before it creates any package or starts PAQ.
- **E2 same-input smoke complete:** `dickens-leading-32k` has frozen input
  SHA-256 `FC42DCB9849222C8704C9DCAE606D075B389B66244FB215035148D6409EC0B31`.
  PAQ8px `-1` produced 9,502 complete archive bytes (2.319824 bpb) and a
  byte-exact round-trip in
  `results/experiments/paq8px-v216-level1-silesia-leading-dickens-32k-e2-20260828`.
  The existing same-input HybridZip Auto evidence is 9,598 bytes (2.343262
  bpb). The 96-byte PAQ advantage applies only to this smoke case.
- **E3 remains gated:** do not automatically start the 36-case PAQ matrix.
  Historical, centred-slice `-1` timings imply about 12 minutes of codec time
  typical and about 30 minutes as a conservative serial reservation. They are
  runtime planning evidence only, not a comparable ratio baseline.

## Experiment Matrix

### Tier A: Prefix continuity and router evidence

- Corpus: all 12 Silesia files; leading 32, 64, and 128 KiB prefixes.
- Reference: PAQ8px v216 `-1`, current R2 `auto`, R2 forced `stored`, and
  forced `zstd` at frozen levels.
- Router: full Auto/oracle, rule-only K=2/K=4/K=8, then ranker K=2/K=4/K=8.
- Every comparison uses identical input bytes and SHA-256.

### Tier B: Separate acceptance corpus

- Complete Silesia and a provenance-approved Tencent corpus.
- OASum is a candidate only. Its approximately 1.065 GB `test.jsonl` and
  CC-BY-SA-3.0 obligations require a separate approval before download.

## Recorded Per Case

`input_sha256`, complete archive bytes/hash, decoded hash, byte-exact status,
codec hash, command, block size, candidate count, selected mode, encode/decode
wall time, P50/P95 latency where available, and peak RAM. Archive bytes always
include headers, CRC, transform metadata, and payload.

## Non-Goals for This First Pass

- No new zstd import: it is already vendored.
- No HZ02 mode renumbering or new archive container solely for encoder policy.
- No GPU implementation before the CPU reference path.
- No Tencent/OASum download, full corpus sweep, or ratio claim until the
  data/license and same-input gates pass.
