# HybridZip R2 Ranker Training-Data Protocol

## Purpose

This protocol turns a completed 32 KiB forced-mode oracle into auditable,
file-level isolated examples for fitting a future K=8 ranker. It does not fit,
install, or promote a model by itself.

The feature vector comes only from the C++ `BlockFeaturesV1` implementation
linked into HybridZip. The prior Python offline preview remains descriptive; it
is not an acceptable feature source for model fitting.

## Inputs

- A `COMPLETE` forced-oracle derivation directory produced by
  `tools/derive_r2_forced_oracle.ps1`, with 32 KiB one-block labels in
  `forced_oracle_rows.csv`.
- The matching source directory containing the original Silesia files.
- `build/Release/hz_r2_feature_dump.exe`, built from the current source.
- An explicit `-ValidationFiles` list. The listed files are wholly held out;
  the exporter rejects an empty holdout, unknown file, duplicate file, or a
  holdout containing every oracle file.

Every source prefix is SHA-256 checked against its forced-oracle row before
feature extraction. This makes rows invalid if the dataset is changed after
the forced ledger completes.

## Export

The guarded preview performs input and split validation without calling the
feature executable or a codec:

```powershell
.\tools\export_r2_ranker_training_set.ps1 `
  -ForcedOraclePath <forced-oracle-directory> `
  -DatasetPath F:\paq8px\silesia `
  -ValidationFiles mozilla,ooffice,reymont,xml `
  -ListOnly
```

After checking the listed plan, export to a new directory:

```powershell
.\tools\export_r2_ranker_training_set.ps1 `
  -ForcedOraclePath <forced-oracle-directory> `
  -DatasetPath F:\paq8px\silesia `
  -ValidationFiles mozilla,ooffice,reymont,xml `
  -OutputPath <new-training-data-directory>
```

The exporter runs `hz_r2_feature_dump` once per labeled input. It performs no
HybridZip archive encode/decode: `codec_invocations` remains zero and
`runtime_started` remains false in both preview and completed metadata.

## Outputs

- `ranker_examples.csv`: one labeled 32 KiB block per file. It stores the
  source hash, complete-oracle byte count, all tied winner modes, the C++ block
  class, current K=8 mode IDs, fixed model identity, and `f00` through `f27`.
- `split.json`: exact training and validation file memberships plus the source
  forced-oracle summary hash.
- `summary.json`: exporter identity, feature-dump hash, ranker identity, row
  counts, and the no-leakage partition declaration.

No row from a validation file may be used during fitting, feature scaling,
threshold selection, model quantization, or hyperparameter selection.

## Model Promotion Boundary

The current `0x00010000` fixed-point model remains bootstrap-only. A fitted
model may replace it only after recording:

1. the complete forced-oracle package and its feature-export summary;
2. the exact file-level train/validation membership;
3. fitting code revision, model bytes, CRC32, SHA-256, feature code revision,
   hard gates, and K threshold;
4. validation recall and archive-byte regret against the held-out oracle; and
5. a separate current-build E5 package meeting the declared K=8 promotion
   gates.

## Verification

`tools/test_r2_ranker_training_set.ps1` builds no archive. It creates two
synthetic 32 KiB sources and tied oracle labels, verifies the no-write preview,
exports 28 C++-computed features per row, and proves one full source file is
in training while the other is in validation.
