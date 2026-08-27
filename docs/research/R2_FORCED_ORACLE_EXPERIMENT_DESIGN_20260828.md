# HybridZip R2 Forced-Mode Oracle Experiment

## Purpose

This protocol produces the missing label source for an evidence-gated K=8
promotion decision. It measures every retained ratio mode on exactly the same
one-block input, then retains every tied minimum complete-archive winner. It
does not use full Auto as a substitute for a forced-mode oracle.

## Fixed Scope

- Inputs: the 12 leading 32 KiB Silesia prefixes in
  `bench/manifests/silesia-leading-32-64-128.tsv`.
- Internal HZ02 block size: 32 KiB. Every recorded input is exactly one HZ02
  block, so an archive-byte winner is a block-level winner.
- Candidate modes: retained HZ02 ratio IDs `0..42`, one forced package per
  mode, plus one Auto reference package. Fast extension ID 43 is excluded:
  it is `ENC_FAST_V1`, not a ratio-router candidate.
- Equality: inputs, executable SHA-256, configuration, block size, and archive
  accounting must match across every forced package.

The ratio oracle is not a PAQ8px comparison and does not establish a CPU/GPU
throughput result. It exists only to label the winner set used by E5.

## Required Evidence

Each forced row must be `COMPLETE/PASS`, use the selected source-prefix
SHA-256, and record exactly one forced block type. Its complete archive bytes
include the HZ02 header, block header, CRC, transform metadata, and payload.

For a block `b` and forced modes `M`:

```text
oracle_bytes(b) = min(archive_bytes(b, m)) for m in M
oracle_winners(b) = { m in M | archive_bytes(b, m) = oracle_bytes(b) }
```

For a shortlist `S`:

```text
tie_aware_hit(b) = oracle_winners(b) intersection S is nonempty
winner_recall = sum(tie_aware_hit(b)) / number_of_blocks
```

The calculation intentionally keeps ties. Replacing tied labels with one
lowest-ID winner would understate recall and bias router fitting.

## Tooling

`tools/run_r2_complete_ledger.ps1` now records an explicit
`block_size_kib` and forwards it to the child runner. Its `-ListOnly` mode
does not create a package or launch the codec.

After the dedicated runtime window is authorized, the 32 KiB forced ledger is
started explicitly, for example:

```powershell
.\tools\run_r2_complete_ledger.ps1 `
  -LedgerId hybridzip-r2-forced-oracle-32k-<id> `
  -ScopesKiB 32 -BlockSizeKiB 32 `
  -AuthorizeRuntimeExperiment
```

The command creates 44 child packages: Auto plus 43 ratio forced modes. It
plans 1,056 codec invocations for 12 inputs, before any retries. It must not
run concurrently with E5 or Fast timing.

`tools/derive_r2_forced_oracle.ps1` performs only ledger validation and label
derivation. It requires a new output directory and has a no-write `-ListOnly`
mode. With a completed matching E5 package it emits:

- `forced_archive_rows.csv`: every forced complete archive result.
- `forced_oracle_rows.csv`: complete-byte minimum and all tied winners.
- `tie_aware_recall_rows.csv`: K2/K4/K8 block hits and regret versus the true
  forced oracle.
- `tie_aware_recall_summary.csv` and `summary.json`: recall and aggregate
  regret by policy.

`tools/run_r2_e5_e6_matrix.ps1` accepts the optional E5-only
`-ForcedOracleLedgerPath`. When supplied for a runtime E5 matrix, it first
performs the derivation tool's read-only forced-ledger validation, records the
normalized ledger path in `experiment.json`, and checks that identity on
`-Resume`. After all E5 child rows pass, it writes the derived artifacts under
`<e5-package>\forced-oracle` and records the linked evidence in the E5
`summary.json`. A completed E5 package requested with this parameter is not
accepted on resume unless that evidence is present and consistent. `-ListOnly`
only displays the planned linkage and launches no codec process.

Example after both packages are complete:

```powershell
.\tools\derive_r2_forced_oracle.ps1 `
  -ForcedLedgerPath <forced-ledger-directory> `
  -E5PackagePath <e5-package-directory> `
  -OutputPath <new-derived-directory> `
  -RequireE5Coverage
```

## Validation Gates

The derivation rejects a package if it finds any of the following:

- missing, duplicate, or non-complete forced mode;
- a mixed executable SHA-256, input identity, scope, or internal block size;
- inputs that are not exactly one internal block;
- incorrect forced block attribution;
- a K2/K4/K8 row with the wrong candidate count or a mixed fixed-point ranker
  identity;
- incomplete E5 coverage when `-RequireE5Coverage` is set.

`tools/test_r2_forced_oracle.ps1` is a no-codec synthetic self-test. It checks
a tied `zstd,fse` oracle and verifies that K2 misses while K4 and K8 hit.

## Promotion Boundary

This tooling makes tie-aware recall measurable; it does not promote K=8 by
itself. Promotion still requires the target recall/regret thresholds, a
no-leakage file-level train/validation protocol for any fitted ranker, and a
separate current-build ratio comparison against PAQ8px on identical inputs.
