# HZ03 External-Core Kill Test Protocol

## Decision Question

Before any HZ03 implementation, determine whether a mature external core can
offer a credible high-ratio foundation on exactly the HybridZip input bytes.
This protocol evaluates a candidate; it does not imply that the candidate is
integrated into HybridZip or licensed for product inclusion.

## Frozen Candidates

| Candidate | Fixed implementation | Initial settings | Role |
| --- | --- | --- | --- |
| `kanzi-l7/l8/l9` | Kanzi 2.5.3, commit `66a80678` | one job, block equals input scope | high-ratio BWT/context-model candidate |
| `libbsc-e2` | libbsc 3.3.12, commit `baffa62c` | one thread, entropy level 2 | block-sorting control |
| `paq8px-l1/l2/l3/l4` | PAQ8px v216, SHA `F7934370...E5F42533` | official numeric level | established high-ratio reference |
| `xz-9e` | XZ 5.6.4 | `-9e`, one thread | LZMA2 control |
| `hybridzip-auto/fast` | current Release | HZ02 R2 policy | current-system controls |

Kanzi and libbsc are independent CLI processes in this experiment. Their
archive bytes include their native headers and checksums. No wrapper header,
shared dictionary, pretrained model, or input-specific side information is
free.

## Staged Execution

1. **K0 smoke:** deterministic random 1 KiB input; every selected candidate
   must create an archive and reconstruct an identical SHA-256.
2. **K1 independent small blocks:** the frozen 12-file Silesia leading-prefix
   manifest at 32/64/128 KiB. Every row is a separate native archive. This is
   the first fair archive-size screen.
3. **K2 superblock screen:** only K1 survivors run on representative full
   Silesia files with 1, 4, and 16 MiB coding blocks. This identifies whether
   short-block state reset is the main cause of the current ratio gap.
4. **K3 acceptance:** a single chosen candidate runs all 12 complete Silesia
   files and then frozen Tencent/OASum validation data, if legal/provenance
   approval exists.

Do not run K2/K3 merely because K0 succeeds. Do not rerun completed E5 or E6
packages.

## Measured Fields

Every retained row records input, archive, and decoded paths, byte counts and
SHA-256 values; encode/decode wall time and sampled peak working set; command
line; exit codes; codec SHA-256; runtime dependency path; and a byte-exact
round-trip status. An archive-size comparison uses the complete native archive
bytes, not the payload alone.

## Gate

A candidate advances from K1 only if every selected row is byte-exact and its
weighted complete archive bytes merit further investigation. The final HZ03
foundation gate remains stricter than bare parity:

```text
candidate archive bytes <= 99.5% of PAQ8px v216 -1
CPU encode >= 0.20 MB/s and decode >= 0.20 MB/s
```

The 0.5% margin reserves room for a future HZ03 container, routing metadata,
and superblock boundaries. K1 alone cannot establish this gate because it
does not cover complete files or Tencent/OASum.

## Known Boundary

libbsc documents a 1 MiB minimum block-size option. Its small-input native
archives are still measured in K1, but that limitation must be retained when
interpreting 32/64/128 KiB results.
