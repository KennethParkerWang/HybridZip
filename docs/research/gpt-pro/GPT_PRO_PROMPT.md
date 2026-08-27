# Copy-Ready GPT Pro Prompt

I need a research decision for an existing C++17 lossless compressor named
HybridZip. Do not restart architecture research or propose a toy rewrite. Read
the attached R2 report and current evidence first, then recommend the next
donor-first implementation that can move the working product toward the stated
ratio and throughput targets.

## Product Requirement

1. Reversible data preprocessing plus category-specific lossless compression.
2. Block/file classification and routing to appropriate compression modules.
3. Coverage for Silesia and an exact Tencent dataset to be selected with
   provenance/license.
4. On both corpora, a weighted complete-archive result better than PAQ8px v216
   `-1` on identical input SHA-256 values.
5. Either 8-10 MB/s end-to-end encode/decode with GPU acceleration, or
   0.16-0.20 MB/s CPU optimization, with 32/64/128 KiB throughput and latency
   analysis.

## Facts You Must Preserve

- HZ01/`PROFILE_V1` must remain decodable.
- HZ02 already has 43 decoder-visible modes and Auto routing. The current
  Silesia leading-32-KiB ledger has 528/528 byte-exact passes. Auto is
  2.028809 bpb and exactly matches the forced-mode archive-byte oracle on 12
  cases, but encodes at only 176.56 B/s.
- The fastest observed high-ratio forced modes are PAQ8px variants at about
  2.1-2.7 KB/s encode, still about 60-75x below the CPU target.
- Current Auto constructs many real candidate archives before choosing the
  smallest. Existing routing uses deterministic structure/family gates, not a
  proven 2-8 candidate shortlist.
- A local PAQ8px `-1` result exists, but it uses centred Silesia slices while
  the R2 ledger uses leading prefixes. Do not compare their ratios as proof.
- No Tencent dataset identity or result package is yet fixed.
- Mature donor source is preferred. For each proposed donor, give upstream URL,
  revision/release, license, source/model size, deterministic decoder risk,
  and exact C++17 integration boundary. New material should be downloaded to
  `E:\MIXER\KU` with provenance before implementation.

## Your Task

Produce a decision-ready research response, not general background.

1. Recommend one architecture and at most two alternatives. Explain whether a
   single profile can plausibly meet both the PAQ8px ratio target and the CPU or
   GPU throughput target. If two profiles are necessary, retain one archive
   format and state which requirement each profile satisfies.
2. Design a deterministic, cheap mode-shortlist router for the existing HZ02
   portfolio. Specify features, compute cost, candidate cardinality, fallback,
   decision coding, pseudocode, and a no-leakage evaluation protocol.
3. Select the next mature donor(s) for a fast reversible path, especially
   block-parallel transforms, LZ matching, and rANS/FSE-compatible coding.
   Reject donors that cannot meet licensing/deterministic-decoder constraints.
4. Design the GPU path concretely: parallel kernels, serial dependencies,
   host-device transfer, 32 KiB launch overhead, exact decoder contract, and
   end-to-end benchmark method. Do not say "use CUDA" without assigning work
   to kernels and a coder.
5. Define preprocessing/classification rules for text, markup/code, records,
   numeric arrays, executables, image/audio-like data, mixed binary, and
   incompressible data. Every transform must have reversible side information
   and must beat complete archive bytes.
6. Provide an ablation/benchmark matrix at 32/64/128 KiB. Require identical
   input hashes, complete archive bytes, byte-exact decode, encode/decode time,
   P50/P95 block latency, peak RAM, candidate count, winner recall, and Auto
   archive-byte regret.
7. Identify and specify the exact Tencent dataset to use: upstream URL,
   license, version/date, files/categories, split, and SHA-256 manifest. If
   several plausible datasets exist, compare them and explicitly mark the
   owner decision required.
8. Finish with a staged source-level implementation plan: files to add/modify,
   smallest lossless test per change, acceptance/rejection condition, and the
   risks that would block the target.

## Evidence Rules

- Never call a result comparable unless input SHA-256 and measurement scope
  match.
- Count headers, metadata, CRC, model IDs, and payload in archive bytes.
- Separate observed measurements from projected performance.
- Do not delete current candidate paths from one 32 KiB matrix.
- Do not treat a shared/pretrained model as free information.
- Cite every external technical claim with an original source URL and license.
