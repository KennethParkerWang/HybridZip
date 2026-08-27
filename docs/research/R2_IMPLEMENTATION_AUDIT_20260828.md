# HybridZip R2 Implementation Audit

## Scope

This audit evaluates the evidence-gated R2 objective against the current
working tree. It distinguishes implementation from measured acceptance. A
passing 1 KiB smoke proves only the named path can round-trip that input.

## Requirement Evidence

| Requirement | Current state | Authoritative evidence | What remains |
| --- | --- | --- | --- |
| Same-input Silesia protocol | Implemented and exercised | `bench/manifests/silesia-leading-32-64-128.tsv`: 36 prefixes, SHA-256 `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D`; E3 package has all 36 matching input/decoded hashes | Execute the corresponding current HybridZip ratio cases. |
| PAQ8px v216 `-1` baseline | Complete | `results/experiments/paq8px-v216-level1-silesia-leading-e3-20260828/`: 36/36 COMPLETE/PASS, 622,563 archive bytes / 2,752,512 input bytes | Current HybridZip comparisons at all three scopes remain pending. |
| K=8 ratio router | 28-feature fixed-point bootstrap and exact feature export implemented | `src/r2/routing/block_features.*`, `mode_ranker.*`, `block_planner.cpp`; CLI `auto-k8`; `hz_r2_feature_dump`; file-level dataset exporter | E5 forced-mode labels, no-leakage fitted model, router-budget timing, and held-out archive/regret measurements. |
| K=2/K=4 ablations | Implemented, ablation only | CLI `auto-k2`/`auto-k4`; current 1 KiB telemetry shows 2/4/8 materialized modes | No policy promotion from this smoke. |
| Candidate accounting | Implemented and guarded | `candidate_modes=<id>:<count>` telemetry; explicit backend-order to BlockMode mapping in `block_planner.cpp` | E5 runtime rows and a matching forced-mode oracle are needed for tie-aware recall. |
| CPU Fast policy | Mode-2 baseline measured; Fast K=4 and Fast-only block executor implemented | `results/experiments/hybridzip-r2-e6-fast-full-20260828-retry1/`: 432/432 byte-exact rows, 324 retained rows, all nine cells above 0.16 MB/s; Mode-43/Fast K=4 smoke; F3 one/two-worker evidence: `results/smoke/r2-f3-fast-executor-1k-20260828-v2/` | Rerun E6 for Fast K=4 with the executor; no post-change throughput claim yet. |
| HZ01 compatibility | Current-build 1 KiB byte-exact gate | `results/smoke/r2-e5-e6-compat-1k-20260828-v1/verification-recovery.json`: 537-byte archive, exact decoded SHA-256 | Larger compatibility regression remains outside the current minimal-test boundary. |
| E5/E6 runtime protocol | E6 complete; E5 pending | `tools/run_r2_e5_e6_matrix.ps1`; E6 package and summary; E5 can bind a completed forced ledger and retain derived tie-aware evidence | E5 full Auto versus K=2/K=4/K=8 measurement and forced-oracle recall evidence. |

## Current Small-Input Gates

The Fast K=4 smoke executable SHA-256 is
`8125425879D70EB24FE5F36379B49A541BE66201319ADF073096FE0DFEF1479B`.

| Policy or profile | Input | Result |
| --- | ---: | --- |
| `auto-k2` | 1,024 bytes | 2 candidates; mode 36; 463-byte archive; exact decode |
| `auto-k4` | 1,024 bytes | 4 candidates; mode 36; 463-byte archive; exact decode |
| `auto-k8` | 1,024 bytes | 8 candidates; mode 36; 463-byte archive; exact decode |
| `fast` (historical baseline) | 1,024 bytes | mode 2/zstd; 662-byte archive; exact decode |
| `fast` (Fast K=4) | 1,024-byte counter | 4 candidates; mode 43/bitshuffle width 2; 159-byte archive; exact decode |
| `fast` (F3 executor) | 1,024 bytes / four 256-byte blocks | one and two workers: same 690-byte archive SHA-256; both exact decode |
| HZ01 | 1,024 bytes | 537-byte archive; exact decode |

`hz_structure_routing_tests.exe` was rebuilt and passed on the current working
tree after the training-data interface was added. It covers deterministic
feature vectors, K=2/K=4/K=8 membership, classification, and the pinned
fixed-point model identity; it does not construct an archive. The broader
`hz_r2_codec_tests.exe` was rebuilt but remains unrun under the minimal-runtime
boundary; the named 1 KiB archive/recovery gates are still the current archive
evidence. No complete CTest matrix was run.

## Offline Preview Boundary

The existing leading-32-KiB current-hash ledger can be reclassified with the
new K=8 rule. The read-only preview in
`results/analysis/r2-complete-ledger/hybridzip-r2-currenthash-cc6d-20260827-r2/derived-k8-offline-20260828-v2/`
has 12 cases, 100% observed winner coverage, and 0 bytes of derived regret.
It is not held-out runtime evidence: it reuses the source ledger and an older
executable hash.

## Runtime Gates

`tools/run_r2_e5_e6_matrix.ps1` defaults to `-ListOnly`. It will refuse to
run unless both switches are provided:

```powershell
-ListOnly:$false -AuthorizeRuntimeExperiment
```

At full scope, each stage plans 12 child packages and 864 encode/decode
invocations. E5 writes full-Auto-reference regret and selected-mode coverage.
It deliberately labels tie-aware forced-oracle recall unavailable unless a
matching complete forced-mode ledger exists. With E5-only
`-ForcedOracleLedgerPath`, it validates the finished 32 KiB forced ledger
before launching child work, stores that normalized path in the experiment
identity, and derives tie-aware evidence after E5 completes. E6 excludes
warmup repeat 0 from its throughput and percentile summary.

## Conclusions Not Yet Supported

- HybridZip does not yet have a complete same-input aggregate comparison with
  PAQ8px v216 `-1` on the current executable at all three scopes. The
  historical 32 KiB Auto ledger is 2,165 bytes larger than E3 PAQ, but it is
  not a current-build 64/128 KiB comparison.
- K=8 has not passed its held-out 99.5% recall and 0.02% regret gates.
- The historical mode-2 Fast baseline meets the CPU 0.16 MB/s floor at all
  measured 32/64/128 KiB input/block-size cells. Fast K=4 plus its Fast-only
  executor has only a deterministic 1 KiB correctness gate; its corpus-level
  throughput measurement and GPU targets are unproven.
- Tencent/OASum coverage, GPU throughput, and final dual-corpus claims remain
  unproven.

## Next Work

E5 is the remaining current-router runtime gate. It must run in a dedicated
window because full Auto materializes PAQ candidates. The matrix runner now
resumes compatible incomplete packages and validates completed packages
without re-running codecs. E3 remains an independent PAQ baseline and must
not be folded into router or Fast results.
