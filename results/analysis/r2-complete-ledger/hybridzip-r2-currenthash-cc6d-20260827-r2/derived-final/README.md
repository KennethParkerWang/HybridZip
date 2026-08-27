# HybridZip R2 Complete Current-Hash Ledger

This derived ledger validates Auto plus all 43 forced HZ02 modes on the exact
file/scope matrix declared by `manifest.tsv`. Every archive byte count comes
from the complete .hz2 file, including the HZ02 header, block headers, CRC32
metadata, backend envelope, and payload. Rows are accepted only when input,
archive, and decoded artifact SHA-256 values match and the decoded bytes equal
the input bytes.

- ledger id: hybridzip-r2-currenthash-cc6d-20260827-r2
- modes: 44 total (Auto + 43 forced)
- rows per mode: 12
- total validated rows: 528
- codec SHA-256: CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191
- source manifest SHA-256: 3795371E9536664F1526D887E547189A8F34858F44E208AD89CD9B4EB568AE82
- Auto gap-positive cases: 0/12
- total Auto gap bytes: 0
- total forced-mode oracle winner rows (ties counted): 12

## Outputs

- `mode_rows.tsv`: normalized archive bytes, timing, memory, and SHA-256 data.
- `per_case_oracle.tsv`: Auto archive bytes versus the minimum complete archive
  bytes among all 43 forced modes for each file and scope.
- `mode_aggregate.tsv`: weighted archive totals, encode/decode time, peak
  memory, Auto selections, oracle wins, and evidence-based recommendation.
- `auto_selection.tsv`: compact Auto/oracle view for review.
- `package_manifest.tsv`: exact package inputs used for this derivation.

`candidate-not-oracle-winner` is a measured retention signal, not permission
to delete donor source. Candidate removal from the product requires a separate
review of corpus coverage, license constraints, and future inputs.