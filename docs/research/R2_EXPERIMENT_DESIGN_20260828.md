# HybridZip R2 Experiment Design

## Purpose

Execute the uploaded R2 decision against the existing HybridZip checkout with
reproducible input identity, complete archive-byte accounting, and byte-exact
decode evidence. The first implementation unit is an encoder-only K=8
shortlist; it does not change the HZ02 container or decoder registry.

## Fixed evidence boundary

- Baseline tag: `baseline-r2-20260828` (`e8a0167502819fc0811d32ed0e78f434d39176d0`).
- Active Release executable SHA-256:
  `CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`.
- Frozen Tier A manifest:
  `bench/manifests/silesia-leading-32-64-128.tsv` (36 rows; SHA-256
  `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D`).
- Existing current-hash R2 ledger: 44 packages, 528/528 byte-exact rows;
  Auto is 99,720 archive bytes over 393,216 input bytes (2.028809 bpb) and
  has a zero Auto/oracle gap on the leading 32 KiB matrix.
- E4 working-tree Release executable SHA-256:
  `03CB6ACD794504C0848B90E2CCA2F724D4A06C86CE8C02EC350E0C95E483D5BD`.
- E6 working-tree Release executable SHA-256:
  `7B8388DB81FCA3994BCE112B7AA712B224CBBCF4C034DDA3765505113334C4FE`.
- Post-K=8-wiring working-tree Release executable SHA-256:
  `C3831DA767B75F06039C52BEA936D8F4DF633E8CB383DE6ECF11D5E8953A9D31`.
  Its focused v3 `auto-k8` evidence is
  `results/smoke/r2-auto-k8-1k-20260828-v3/verification.json`: 1,024 input
  bytes, 463 complete archive bytes, eight materialized candidates, mode 37,
  and a byte-exact decoded SHA-256.
- E5-ablation working-tree Release executable SHA-256:
  `E65526F9DFF3F93844E004D63C7B2A4E4F219B5EAB3F1B3D3ABCF0B301F65003`.
  It adds encoder-only K=2/K=4 policies and candidate-mode telemetry. The
  1 KiB ablation smoke is byte-exact for K=2/K=4/K=8, but is not a corpus
  result.

These values are continuity evidence, not a PAQ8px corpus claim. The one
same-input PAQ smoke is separately recorded and remains a single-case result.

## Tier A matrix

| Stage | Inputs | Variants | Required evidence |
| --- | --- | --- | --- |
| E3 | 12 Silesia files x 32/64/128 KiB leading prefixes | PAQ8px v216 `-1` | complete archive bytes, archive/input/decoded hashes, byte-exact PASS |
| E4 | Frozen fixtures and one 1 KiB smoke | `auto-k8` | deterministic features, eight-mode shortlist, exact round trip |
| E5 | Held-out frozen prefixes | full `auto`, rule K=2/K=4/K=8, matching 32 KiB forced oracle | tie-aware winner recall, regret bytes/percent, candidate count, latency/RAM |
| E6 | Same inputs at 32/64/128 KiB | `fast` policy using zstd level 3 | complete archive bytes, P50/P95 latency, encode/decode MB/s, RAM |

E3 is a serial PAQ run and is not started by this implementation checkpoint.
E5 cannot promote K=8 until it has held-out measurements. E6 is a separate
throughput policy and must not be used to claim PAQ-level ratio.

The `fast` policy is deliberately additive at the encoder-policy layer. It
uses the existing HZ02 zstd mode (mode 2) and clamps the zstd level to 3 for
predictable CPU cost; no new archive mode or decoder path is introduced.

## K=8 rule

For every nonempty block, the encoder requests these mandatory candidates:

1. `Stored`
2. `Zstd`
3. `Paq8pxGenericSse`
4. `Paq8pxDetectedSse`

The remaining four are selected by deterministic integer byte features:

| Class | Additional candidates |
| --- | --- |
| text | `Ppmd7`, `Ppmd8`, `BrotliText`, `BwtZstd` |
| x86 | `Fse`, `Lzma`, `X86BcjZstd`, `Bcj2Zstd` |
| numeric | `Fse`, `Lzma`, `DeltaZstd`, `ShuffleZstd` |
| generic | `Fse`, `Lzma`, `Ppmd7`, `Ppmd8` |

Features are byte count, printable/whitespace/markup/zero per-mille,
lag-1/2/4/8 equality per-mille, x86 branch density per-mille, and unique-byte
count. No floating-point operation or archive metadata is used by the ranker.

## E5 ablation policies and telemetry

`auto-k2` and `auto-k4` are ablation policies only; neither is a promoted
ratio policy. They reuse the HZ02 container and all decoder-visible IDs.

| Policy | Candidates |
| --- | --- |
| `auto-k2` | Stored, `paq8px-generic-sse` |
| `auto-k4` | Stored, zstd, `paq8px-generic-sse`, `paq8px-detected-sse` |
| `auto-k8` | The eight candidates specified above |

The CLI emits `full_oracle=0|1`, `candidate_modes=<id>:<block-count>`, and
the fixed-point `ranker_version`, `ranker_crc32`, and `ranker_sha256` as
encoder telemetry. This telemetry is not archive metadata. It lets E5 prove
exactly which candidates and which frozen model were materialized for a
shortlist. The matrix runner writes the model identity to every row and
rejects a completed package containing more than one identity. A full Auto
reference can measure archive regret and selected-mode coverage. The new
32 KiB one-block forced-oracle tooling can derive tie-aware recall once its
matching ledger is completed; see
`docs/research/R2_FORCED_ORACLE_EXPERIMENT_DESIGN_20260828.md`.

## Acceptance formulas

```text
bpb = 8 * complete_archive_bytes / input_bytes
regret_bytes = shortlist_complete_bytes - full_oracle_complete_bytes
regret_percent = regret_bytes / full_oracle_complete_bytes
winner_recall = blocks where shortlist contains any tied oracle winner / total blocks
```

K=8 is eligible for promotion only if held-out tie-aware winner recall is at
least 99.5% and aggregate regret is at most 0.02% of full-oracle bytes. The
current prefix ledger does not satisfy this gate by itself because it records
the full Auto/oracle, not a K=8 held-out measurement.

## Next execution targets

1. E5: build a non-overwriting held-out K=2/K=4/K=8 ledger from the frozen
   manifest. For every block, retain the full-Auto complete-archive oracle,
   shortlist complete archive bytes, recall, regret, candidate count, wall
   time, and peak RAM. Run the separate same-executable 32 KiB forced oracle
   and derive tied winner labels before reporting recall. A K=8 promotion
   claim is prohibited until its stated recall and regret gates pass.
2. E6: use the Fast policy at 32, 64, and 128 KiB with three retained timing
   repeats after a warmup. Report encode/decode MB/s, P50/P95 block latency,
   peak RAM, byte-exact reconstruction, and complete archive bpb. The CPU
   gate is at least 0.16 MB/s encode and decode for each block size.
3. E3: retain the same-input PAQ8px v216 `-1` matrix as an independent serial
   baseline. It must cover all 36 frozen prefix cases before any aggregate
   HybridZip-versus-PAQ ratio statement.
4. E7: do not download OASum or report Tencent coverage until the owner
   approves the named version, the approximately 1.065 GB test artifact, and
   the CC-BY-SA-3.0 compliance boundary.

`tools/run_r2_e5_e6_matrix.ps1` implements the guarded matrix protocol. It
defaults to `-ListOnly`; runtime requires both `-ListOnly:$false` and
`-AuthorizeRuntimeExperiment`. E5 plans 12 child packages (four policies by
three block sizes). E6 plans 12 child packages (one Fast policy, three block
sizes, one warmup plus three retained repeats). Both plans contain 864
encode/decode invocations at the full 12-file x 32/64/128 KiB scope.

## Runtime and safety boundary

- This checkpoint runs focused compilation and one 1 KiB `auto-k8` round trip.
- It does not run the 36-case PAQ matrix, full Auto/D40 sweeps, CTest, OASum
  download, or 32/64/128 KiB runtime batches.
- All new output packages must be unique and non-overwriting.
- HZ01/`PROFILE_V1` and all existing HZ02 mode IDs remain compatibility gates.
