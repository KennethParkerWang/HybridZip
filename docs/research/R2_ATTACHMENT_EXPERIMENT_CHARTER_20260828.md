# HybridZip R2 Attachment Experiment Charter

## Decision and Scope

This charter turns the attached R2 research decision into executable evidence
gates. It preserves one HZ02 container, HZ01 decoding, HZ02 mode IDs `0..42`,
and the append-only Fast extension at ID `43`.

`ENC_RATIO_V1`, `ENC_FAST_V1`, and `ENC_ORACLE` are separate measurements.
Success on one track must not be reported as success on another track.

## Frozen Inputs and Evidence Rules

| Item | Value |
| --- | --- |
| Tier-A corpus | 12 Silesia files, leading 32/64/128 KiB prefixes |
| Manifest | `bench/manifests/silesia-leading-32-64-128.tsv` |
| Manifest SHA-256 | `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D` |
| Ratio reference | PAQ8px v216 `-1`, same manifest bytes |
| Router labels | 32 KiB input and 32 KiB internal block, all 43 forced ratio modes |
| Fast blocks | 32, 64, and 128 KiB internal blocks |
| Default correctness gate | Byte-exact decoded SHA-256 for every retained row |

Every retained result must include the input, executable, archive, and decoded
SHA-256 values; complete archive bytes; command; source revision; block size;
and an `environment.json` fingerprint. Archive bytes include HZ02 framing,
CRC, extension metadata, transform side information, and payload.

## Objectives and Gates

| ID | Objective | Exact gate | Status |
| --- | --- | --- | --- |
| G0 | Protect compatibility | HZ01 plus retained HZ02 golden decodes; malformed/truncated metadata rejection | Existing focused evidence; rerun after wire-format changes |
| G1 | Build current ratio labels | Complete 12-file, 32 KiB forced oracle; 43 forced modes plus Auto | Complete; current executable and all 528 forced/Auto rows validate byte-exactly |
| G2 | Validate the K=8 ratio router | Held-out file-level labels; tie-aware recall >=99.5%, byte-weighted recall >=99.9%, aggregate regret <=0.02%, P95 regret <=16 bytes | E5 complete for 432 matrix rows; matching forced-oracle labels cover 36 32 KiB rows and K=8 is 12/12 there; full held-out promotion remains pending |
| G3 | Evaluate K=2/K=4 ablations | Complete-byte regret and category representation; promote K=4 only when it uses <=25% of the PAQ byte margin | Complete as an ablation: E5 reports full-byte regret and selected-mode coverage; no production promotion claimed |
| G4 | Re-measure Fast K=4/executor | All 32/64/128 KiB scope/block cells byte-exact, encode and decode >=0.16 MB/s | Complete for single worker: 432/432 rows and all nine cells pass; second-worker scaling is optional |
| G5 | Establish Silesia ratio acceptance | On complete files, `sum(HybridZip archive bytes) < sum(PAQ8px archive bytes)` for the same input hashes | Pending G2 and full-corpus policy run |
| G6 | Establish Tencent text/records evidence | Complete OASum `test.jsonl` with a frozen manifest and separate result table | Blocked on owner/legal approval for 1,065,019,104-byte CC-BY-SA-3.0 input |
| G7 | GPU Fast path | CPU reference decoder plus end-to-end encode/decode >=8 MB/s at all three sizes | Deferred until G4 fails or owner requires GPU |

The CPU Fast objective is 0.16 MB/s, not a ratio claim. The PAQ comparison is
strict: equality is not an improvement. OASum and Silesia are reported
separately; no cross-corpus aggregate is permitted without corpus weights.

## Execution Sequence

### E4: Current-Build Forced Oracle (G1)

- Input: all 12 frozen leading 32 KiB prefixes, internal blocks fixed at 32 KiB.
- Methods: `auto` plus 43 forced ratio modes.
- Workload: 44 child packages and 1,056 encode/decode codec invocations before
  retries.
- Output: `results/analysis/r2-complete-ledger/<ledger-id>/` and child packages
  under `results/experiments/`.
- Pass condition: every mode/file row is `COMPLETE/PASS`, identities agree,
  and the oracle derivation accepts the ledger.
- Rejection: any missing mode, mixed executable/input/block identity, or
  non-byte-exact decode invalidates the ledger.

### E5: No-Leakage Router Evaluation (G2/G3)

1. Export C++ feature vectors and tied forced winners using a file-level
   holdout split.
2. Fit a candidate model using training files only. Candidate model bytes stay
   uninstalled until held-out gates pass.
3. Run `auto`, `auto-k2`, `auto-k4`, and `auto-k8` on the frozen matrix.
4. Attach the completed forced ledger and derive tie-aware recall/regret.

For the full 12-file x three-prefix x three-block-size matrix, E5 plans 864
codec invocations. It is PAQ-heavy and must not run concurrently with E6.

### E6: Post-Change Fast Policy (G4)

- Methods: Fast K=4 with one worker first, then the selected fixed worker
  count.
- Repeats: one warmup and three retained repeats.
- Matrix: 12 files x three prefix sizes x three internal block sizes.
- Measurements: complete bytes, encode/decode throughput, raw per-block
  queue-plus-service and service-only nanosecond samples, and P50/P95 derived
  from those samples.
- Workload: 432 retained/warmup rows, each containing encode and decode.

Keep transform variants only when their complete HZ02 bytes improve on the
raw Fast extension while their latency remains in policy budget.

### E7: Complete-Corpus Acceptance (G5/G6)

Run complete Silesia and, only after a separate owner decision, complete OASum
`test.jsonl`. PAQ8px receives the identical complete bytes. Preserve all
archives, logs, manifests, environment files, and result summaries.

### E8: Conditional GPU Work (G7)

Implement and test the CPU reference decoder before any CUDA acceptance claim.
Use `LZ_RANS_V1` only through Fast extension codec ID 1. Report kernel-only,
transfer, and end-to-end times separately. Single 32 KiB blocks are cold
latency data, not a throughput configuration; bulk testing batches at least
512 KiB and targets 4 MiB.

## Current Execution Boundary

This turn starts E4 with a no-codec preflight only. The preflight confirms the
current executable path, fixed mode count, frozen file count, block size, and
the exact 1,056-invocation cost without creating an experiment package.

Starting E4 runtime requires the explicit command switch
`-AuthorizeRuntimeExperiment`. It must use a new non-overwriting ledger ID and
remain resumable only on the same environment fingerprint. Do not replace E4
with a subset and label it as G1 evidence.

## Known Non-Acceptance Evidence

- The committed offline K=8 preview is a read-only derivation from an older
  12-file ledger. It is labelled `PREVIEW` and cannot promote K=8.
- The prior 432-row Fast result is a mode-2 baseline. It cannot establish the
  current Fast K=4/executor performance.
- The staged zstd v1.5.7 donor is provenance evidence. Production currently
  uses vendored zstd 1.6.0, so current Fast measurements remain 1.6.0 results.
