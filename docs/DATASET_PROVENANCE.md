# Dataset Provenance And Distribution Boundary

## Silesia

- Official corpus page: `https://sun.aei.polsl.pl/~sdeor/index.php?page=silesia`
- Official archive endpoint: `https://sun.aei.polsl.pl/~sdeor/corpus/silesia.zip`
- Endpoints verified reachable: 2026-08-20.
- Canonical Experiment Ledger path: `F:\paq8px\silesia`.
- Read-only project mirror: `E:\MIXER\silesia`.
- Local identity check: all 12 complete files match between the two paths by
  SHA-256.

The official corpus page identifies a separate origin for each member, but it
does not present one corpus-wide redistribution license. This project therefore
does not assert a right to republish the corpus bytes.

The formal Experiment Ledger package contains local evidence copies of 36
input prefixes and their decoded equivalents. Treat the entire package as a
local research artifact. Before distributing it, confirm the applicable terms
for every Silesia member or exclude the corpus-derived `inputs/`, `decoded/`,
and archive artifacts. `experiment.json` and `results.csv` contain identities
and measurements, not sufficient permission to redistribute source bytes.

The repository `.gitignore` excludes `inputs/`, `archives/`, `decoded/`, and
`logs/` below every generated experiment package. The complete files remain on
this machine for validation, while a normal `git add .` stages only the package
metadata/results unless the user explicitly overrides the ignore boundary.

The downloaded audio product-test input has separate provenance at
`E:\MIXER\KU\product-corpus\SOURCE_PROVENANCE.md` and is not copied into the
HybridZip source repository.
