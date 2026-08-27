# HybridZip: Research Brief for Ratio and Throughput Targets

## Decision Context

HybridZip R2 already has a runnable C++17 HZ02 portfolio: 43 decoder-visible
coding paths, reversible representations, structure gates, specialist paths,
and an Auto chooser that compares complete archive bytes. The next research
task is not another architecture rewrite. It is to choose and justify the
smallest set of algorithmic changes that can move this implementation toward
the required compression-ratio and hardware-throughput targets.

The central tension is measured: current Auto reaches the best observed
archive size by evaluating a broad candidate portfolio, but is far too slow for
the requested CPU throughput. A proposal that improves only ratio or only
throughput is incomplete.

## Required R&D Objectives

The following are product requirements supplied by the project owner.

1. Implement a high-ratio lossless compression flow containing reversible data
   preprocessing and data compression for different data categories.
2. Classify data blocks/files and route each to an appropriate compression
   module.
3. Cover Silesia and a Tencent dataset with hardware-friendly model structure.
4. On both named corpora, beat PAQ8px v216 `-1` in average compression result.
5. Achieve either 8-10 MB/s encode and decode with GPU acceleration, or
   0.16-0.20 MB/s with CPU optimization. Analyze throughput and latency at
   32, 64, and 128 KiB block granularities.

For acceptance, interpret "better compression ratio" as a smaller weighted
complete-archive fraction over the same inputs:

```text
archive_fraction = sum(complete archive bytes) / sum(input bytes)
bpb              = 8 * sum(complete archive bytes) / sum(input bytes)
```

All headers, transform metadata, CRC, model identifiers, and payload bytes
must be included. Every measured row must be byte-exact after decoding. A
simple average of per-file ratios is not the acceptance metric.

The stated MB/s target is treated as decimal MB/s (1,000,000 bytes/s) until
the owner explicitly chooses MiB/s instead. GPU reporting must provide both
end-to-end throughput including host-device transfers and kernel-only
throughput; only the former can satisfy a product target.

## Verified Current State

### Implementation

- HZ01 remains compatible: NGram, PPMD, Match, Online LSTM, adaptive mixer,
  arithmetic coding, and the `PROFILE_V1` archive contract.
- HZ02 implements modes `0..42`: stored, generic predictors, LZ/entropy
  coders, BWT/BCJ/numeric representations, text/media specialists, PAQ8px
  branches, neural-profile branches, and multi-coder paths.
- Auto has three source-integrated layers: byte-only structure activation,
  family-level activation, then candidate selection by complete archive bytes.
- The active Windows Release executable has SHA-256
  `CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`.

### Current R2 Evidence

Ledger: `hybridzip-r2-currenthash-cc6d-20260827-r2`.

| Scope | Result |
| --- | --- |
| Corpus | 12 Silesia files, leading 32 KiB prefix each |
| Packages | Auto plus 43 forced modes = 44 packages |
| Validated rows | 528/528 `COMPLETE/PASS`, byte-exact SHA-256 round trips |
| Auto archive bytes | 99,720 over 393,216 input bytes |
| Auto compression | 0.253601 archive fraction, 2.028809 bpb |
| Auto vs forced oracle | 0 bytes aggregate gap; 0 bytes on all 12 cases |
| Auto winners | `paq8px-generic-sse` on 7 cases; `paq8px-detected-sse` on 5 |
| Auto encode/decode time | 2,227.074 / 151.529 seconds aggregate |
| Auto peak sampled RAM | 735.305 MiB |

The claim is strictly limited to this one-run, leading-32-KiB Silesia matrix.
It is not a full-Silesia result, not a Tencent result, and not evidence that
the other 41 donors should be removed.

### Measured Speed

| Mode | Encode B/s | Decode B/s | Encode MB/s | Decode MB/s | Peak RAM MiB |
| --- | ---: | ---: | ---: | ---: | ---: |
| Auto | 176.56 | 2,594.99 | 0.000177 | 0.002595 | 735.305 |
| `paq8px-generic-sse` | 2,131.54 | 2,129.40 | 0.002132 | 0.002129 | 551.176 |
| `paq8px-detected-sse` | 2,666.89 | 2,680.04 | 0.002667 | 0.002680 | 734.023 |

Against the 0.16 MB/s CPU floor, the observed encode gap is about 906x for
Auto and 60x for the fastest observed forced PAQ8px mode. Auto decode is
about 62x below that floor. These are aggregate engineering observations, not
repeat-run estimates.

## What Is and Is Not a PAQ8px Comparison

A local PAQ8px v216 `-1` reference package exists at
`F:\paq8px\benchmark_paq8px_32KiB_parallel\`. It reports 2.045369 bpb over
12 Silesia 32 KiB **centred** slices, with 59.350 seconds aggregate encoding
time and 58.422 seconds aggregate decoding time. This is useful for code and
protocol study.

It is not valid acceptance evidence against the current R2 ledger: R2 used
the **leading** 32 KiB prefix, and the PAQ package used a centred slice with
different input SHA-256 values. A fair ratio comparison must first generate
PAQ8px v216 `-1` archives for the exact R2 input artifacts at 32, 64, and 128
KiB, or regenerate both compressors from one shared manifest.

The Tencent dataset is not present as a named local corpus, provenance
manifest, or result package. Its exact public source, version, license, member
list, splits, and SHA-256 manifest remain a prerequisite for a dual-corpus
claim.

## Diagnosed Engineering Problems

### P0: Auto selects by exhaustive archive construction

`src/r2/block/block_planner.cpp` constructs generic candidates and many
structure-gated candidates, then compares their actual payload/metadata sizes.
`BlockPlanner::plan` retains a 43-entry candidate array and counts each
materialized candidate. This gives the observed 0-byte oracle gap, but it
spends expensive encoder work before writing one winner.

The existing router is not yet a proven production shortlist. In
`src/r2/routing/activation_router.cpp`, activation is a deterministic set of
hand-set feature thresholds plus a stale-loss family gate. It has not been
evaluated for recall of the archive-byte winner, candidate-count reduction, or
throughput at 32/64/128 KiB.

### P0: Ratio target and throughput target lack a shared Pareto design

PAQ-like bitwise context mixing and arithmetic coding are naturally serial and
memory-heavy. They are plausible high-ratio candidates, but are a poor direct
fit for 0.16-0.20 MB/s CPU and especially for GPU throughput. The research must
identify a reversible fast path using block-parallel operations and a parallel
entropy backend such as rANS/FSE, while preserving a high-ratio path where it
actually wins.

Do not assume that a GPU automatically accelerates the current bitwise PAQ
loops. A credible GPU design must identify parallel kernels, state boundaries,
model transfer cost, entropy-coding strategy, determinism, and the exact
decoder hardware contract.

### P0: No fair acceptance benchmark yet

The result importer requires 12 Silesia files x 32/64/128 KiB x variant x
repeat, with complete archive bytes, timing, peak RAM, commands, exit codes,
and input/archive/decoded SHA-256. Current R2 has only leading 32 KiB results.
No same-input PAQ8px `-1` baseline or Tencent suite currently closes the
target.

### P1: Preprocessing must earn its metadata and latency

R2 contains BWT, BCJ, delta, shuffle/bitshuffle, record, image, audio, and
text transforms. A new preprocessor must specify a deterministic applicability
test, side-information format, inverse procedure, metadata size, and latency.
It stays only if complete archive bytes improve against its no-transform route.

### P1: Hardware-friendly contract is underspecified

No target GPU model, CPU model/core count, RAM budget, thread count, batch
size, cold/warm-cache policy, or latency percentile has been fixed. Research
should propose a concrete minimum test platform, then engineering must freeze
it before accepting throughput claims.

## Research Questions for GPT Pro

Rank proposals by expected ratio gain per added latency and implementation
risk. Do not return a generic list of codecs.

1. What CPU-oriented architecture can retain PAQ8px-level ratio while meeting
   0.16-0.20 MB/s for 32/64/128 KiB blocks? State whether a single profile can
   plausibly meet both goals; if not, design interoperable `ratio` and `fast`
   profiles without weakening the stated acceptance requirement.
2. Which smallest deterministic feature vector can choose a 2-, 4-, or 8-mode
   HZ02 shortlist with high oracle-winner recall? Include feature cost,
   classifier/router form, encoding of router decisions, fallback policy, and
   a training/validation protocol that does not leak archive bytes.
3. Which existing mature donor components should be downloaded or ported next
   for a fast, GPU-friendly reversible path? Prefer code with a feasible
   license and deterministic decoder. Give upstream URL, revision, license,
   core files, C++17 integration boundary, and why it improves the measured
   bottleneck.
4. Which preprocessing families should be retained or extended for Tencent-like
   data classes (text, JSON/XML, source, binary executables, records/numeric,
   image/audio, already-compressed data)? For each, state a cheap detection
   rule and expected candidate modes.
5. If using GPU, which parts run in parallel (feature extraction, transforms,
   LZ matching, neural inference, rANS/FSE), which remain serial, and how are
   exact decode and cross-device determinism guaranteed? Account for transfer
   and launch overhead at 32 KiB.
6. Which ablations can distinguish a useful router/preprocessor from an
   improvement caused only by more candidate evaluation or a larger model?
7. Specify a Tencent dataset selection process: original URL, license,
   version/date, categories, train/validation/test partition, and a hash
   manifest. Do not substitute an unrelated Tencent-branded dataset silently.

## Required Research Deliverables

The research answer should produce the following decision-ready artifacts.

1. One recommended architecture, plus at most two ranked alternatives.
2. A donor table: upstream URL, commit/release, license, required source/model
   size, GPU/CPU suitability, decoder determinism risk, and exact module
   boundary in `src/r2/`.
3. A mode-shortlist design with pseudocode and an experiment to measure
   shortlist recall, Auto archive-byte regret, candidate count, memory, and
   32/64/128 KiB latency.
4. A preprocessing/classification map from data features to candidate families.
5. A benchmark specification that compares identical input SHA-256 values for
   HybridZip, PAQ8px v216 `-1`, zstd, and one fast hardware-oriented baseline.
6. A staged code plan listing specific files to add/change, the smallest
   byte-exact test for each stage, and the result condition that keeps or
   rejects the change.
7. A risk list that separates facts from projections. It must state whether
   the throughput target is likely CPU-feasible, GPU-feasible, or requires a
   negotiated separate product profile.

## Acceptance Benchmark to Implement After Research

| Step | Inputs | Variants | Required outcome |
| --- | --- | --- | --- |
| A | Fixed Silesia manifest, 12 files x 32/64/128 KiB | PAQ8px v216 `-1`, current R2, proposed variants | Identical input SHA-256, full archive bytes, byte-exact round trips |
| B | Fixed Tencent manifest with recorded license/version/hash | Same variants | Same protocol and per-category breakdown |
| C | Both corpora | shortlisted Auto cardinalities 2/4/8 plus full Auto oracle | Ratio regret, winner recall, candidate count, latency, RAM |
| D | Both corpora, 32/64/128 KiB | selected CPU profile | Encode/decode MB/s, per-block P50/P95 latency, peak RAM |
| E | Same, if GPU profile exists | end-to-end and kernel-only measurements | 8-10 MB/s product path or a documented failure against the target |

For every `COMPLETE/PASS` result retain complete archive files, decoded files,
stdout/stderr, commands, exit codes, and SHA-256 values. Use the Experiment
Ledger import format rather than a summary-only spreadsheet.

## Boundaries

- Preserve HZ01/`PROFILE_V1` decode compatibility.
- Retain donor-first implementation: download mature source under
  `E:\MIXER\KU`, record URL/revision/license/SHA-256, and do not recreate a
  mature algorithm from a paper when source is available.
- Do not delete the 41 current non-winning paths solely from the leading-32-KiB
  ledger. Their contribution on other sizes, categories, and Tencent data is
  unknown.
- Do not count a pretrained/shared model as free compression information.
  Declare model availability, hash, transmitted bytes, and decoder contract.
- Do not claim the ratio target, hardware friendliness, GPU support, or
  Tencent coverage until the acceptance benchmark above passes.
