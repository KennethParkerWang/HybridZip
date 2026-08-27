# HybridZip R2 Fixed-Point Ranker V1

## Status

This is the encoder-only bootstrap ranker used by `auto-k8`. It is a
deterministic implementation checkpoint, not a trained or promoted router.
Its values must not be used to claim held-out recall, regret, PAQ8px ratio, or
CPU throughput until E5 produces the forced-mode label matrix.

## Feature Contract

`BlockFeaturesV1` exposes exactly 28 integer features. Fraction and entropy
features use Q12; block-size bucket, selected periodic width, maximum sampled
LZ match length, and packed flags are discrete integers.

| IDs | Feature group |
| --- | --- |
| F0 | 32/64/128 KiB block-size bucket, otherwise zero |
| F1-F3 | Byte entropy, 16-class coarse entropy, maximum byte frequency |
| F4-F8 | Zero, FF, printable, high-bit, whitespace fractions |
| F9-F12 | Newline, digit, JSON/XML punctuation, source punctuation fractions |
| F13-F15 | Equal-adjacent, runs >=4 coverage, capped log2 longest run |
| F16-F19 | Small-delta scores for widths 1, 2, 4, and 8 |
| F20-F22 | Low-byte concentration, best periodicity score, selected width |
| F23-F25 | Sampled LZ coverage, mean length, maximum length |
| F26 | Plausible in-block x86 E8/E9 relative-target density |
| F27 | UTF-8, known magic, compressed magic, saturation, and OOD flags |

The extractor uses one 256-entry histogram and one 4,096-entry last-position
table. LZ samples one four-byte sequence per eight input bytes and compares at
most 32 bytes per probe. It uses only integer operations; entropy uses a fixed
Q12 mantissa table plus integer leading-bit count.

## Model Contract

`FixedPointRankerModelV1` has the specified 2,644-byte in-memory layout for
the retained HZ02 IDs `0..42`:

```text
weight[43][28]  int16   2,408 bytes
bias[43]        int32     172 bytes
feature_shift[28] int16    56 bytes
version + CRC32  uint32      8 bytes
total                       2,644 bytes
```

The current bootstrap version is `0x00010000`. The CRC32 is calculated over
the canonical little-endian weights, biases, shifts, and version at process
startup, then verified before shortlist construction. The canonical image also
includes that CRC32 as its final four bytes and is SHA-256 identified in every
R2 encoder telemetry line as `ranker_version`, `ranker_crc32`, and
`ranker_sha256`. The E5/E6 runner persists these three fields in every matrix
row and rejects a completed package with more than one model identity. Scores
accumulate in signed 64-bit integers; score ties use lower mode ID. Mode 43
remains a Fast policy extension and is intentionally outside this 43-row ratio
model.

The ranker keeps Stored plus the two observed PAQ8px-SSE winners mandatory.
The remaining K=8 slots are filled by scored generic-LZ, applicable
representation, applicable specialist, high-ratio, and globally applicable
families. Hard class gates run before score sorting, so text-specialist bias
cannot activate a text transform for x86/numeric blocks.

## Evidence

- `hz_structure_routing_tests.exe` checks feature-vector repeatability,
  compressed-signature flags, classifier coverage, model version, model CRC,
  the pinned canonical SHA-256, quota cardinality, and duplicate-free
  shortlists.
- Frozen bootstrap identity: version `0x00010000`, CRC32 `0x1025B343`,
  SHA-256 `4B1AC26C40AD4DA50312FD3B694D7E636FB768C2336FE773BC82D36424C27A4B`.
- `results/smoke/r2-f1-model-identity-1k-20260828-v2/verification.json`
  records the identity telemetry and a byte-exact 1,024-byte `auto-k8` round
  trip with a 1,084-byte complete archive.
- `results/smoke/r2-f1-fixed-ranker-auto-k8-1k-20260828-v1/verification.json`
  records a current executable 1 KiB byte-exact `auto-k8` round trip: eight
  candidates `0,2,3,4,27,28,36,37`, stored selected.

## Training Boundary

The bootstrap weights are hand-set, deterministic initialization values. The
only permitted promotion path is:

1. Run a complete forced-mode matrix that records complete HZ02 archive bytes
   and input/decoded SHA-256 for each block.
2. Derive tied oracle labels and archive-byte regret, not mode-ID accuracy.
3. Fit using Silesia file-level leave-one-file-out partitions; never randomly
   split blocks from the same source file.
4. Freeze the model bytes, SHA-256, CRC32, feature code revision, hard gates,
   K thresholds, and exact train/validation membership before held-out E5.
5. Require the protocol's K=8 recall/regret gates before changing the policy
   status from experimental.
