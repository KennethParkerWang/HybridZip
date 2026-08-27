# HybridZip R2 Experiment Protocol

## Purpose

This protocol operationalizes the user-provided R2 decision without replacing
the existing HZ02 portfolio. It separates proof of compression ratio from
proof of throughput and prevents an encoder proposal from being reported as
accepted before archive-byte evidence exists.

## Invariants

- HZ01 archives continue to decode.
- Existing HZ02 IDs `0..42` retain their numeric values and behavior.
- New decoder-visible functionality is append-only, beginning with ID `43`.
- Each measured row retains input SHA-256, executable SHA-256, complete
  archive bytes/SHA-256, decoded SHA-256, command, and byte-exact outcome.
- Every new E5/E6/forced-oracle package includes `environment.json`, recording
  CPU, RAM, GPU/driver when discoverable, active power plan, compiler versions,
  codec hash, source revision, and a stable fingerprint. Resume rejects a
  matrix or forced ledger when the current fingerprint differs.
- `bpb = 8 * complete_archive_bytes / input_bytes`.

## Current Evidence

| Item | Frozen scope | Confirmed result | Boundary |
| --- | --- | --- | --- |
| E3 PAQ8px v216 `-1` | 12 Silesia leading prefixes at 32/64/128 KiB | 36/36 byte-exact; 1.809440 bpb aggregate | Current HybridZip 64/128 KiB comparison is absent |
| E5 router | K=2/K=4/K=8 against full Auto | Guarded runner and 1 KiB checks exist | Full matrix is not run; no K=8 promotion |
| E6 Fast baseline | Same 36 inputs, three block sizes, 1 warmup + 3 retained repeats | 432/432 byte-exact; min encode/decode 0.6977/0.6476 MB/s | Existing mode-2 zstd only; K=4 rerun pending |
| Fast K=4 smoke | Deterministic 1 KiB counter input | 4 candidates; Mode 43 selected; bitshuffle width 2; 159-byte archive; byte-exact | Smoke only, no corpus throughput claim |

## Ratio Track: ENC_RATIO_V1

1. Retain full `auto` as oracle-only encoder.
2. Measure K=2, K=4, and K=8 on identical frozen inputs.
3. Record selected mode, candidate count, archive bytes, and regret against
   full Auto.
4. Do not promote K=8 without forced-mode oracle evidence for tie-aware winner
   recall. Intended gates: 99.5% block recall, 99.9% byte-weighted recall,
   <=0.02% aggregate regret, and <=16-byte P95 regret.
5. HybridZip must produce fewer complete archive bytes than PAQ8px on each
   matching corpus/scope; equality is not improvement.

## Fast Track: ENC_FAST_V1

1. Add `MODE_FAST_EXT_V1` at ID 43 with metadata `version=1`, `codec=zstd`,
   `transform=none`, `uLEB128(side_info_bytes)=0`, and a zstd frame payload.
2. HZ02 continues to own raw length and CRC. The extension omits zstd
   checksum, content-size, and dictionary-ID flags and uses one worker.
3. Add shuffle, bitshuffle, delta/XOR, and BCJ one at a time. Keep a transform
   only if its complete HZ02 block is smaller than raw extension zstd.
4. Fast K=4 is now formed after raw extension, transform comparison, and the
   independent 1 KiB decode/malformed-metadata gates. Keep it experimental
   until the post-change 32/64/128 KiB matrix is rerun.
5. Repeat the 32/64/128 KiB matrix after policy changes. CPU acceptance needs
   byte-exact reconstruction and >=0.16 MB/s encode/decode in every cell.

## Execution Order

| Priority | Work item | Immediate proof | Not claimed yet |
| --- | --- | --- | --- |
| P0 | ID-43 raw zstd extension | Build, forced 1 KiB decode, malformed-metadata rejection, HZ01 smoke | v1.5.7 production acceptance |
| P1 | Fast transforms and K=4 | Per-transform archive-byte comparison and inverse-tail tests | Corpus-level Fast result |
| P1 | 28-feature ranker and E5 | Bootstrap feature/model implementation; then frozen trained model identity and held-out coverage/regret | K=8 promotion without forced oracle |
| P1 | Block executor | Canonical order, repeatable bytes, Fast timing rerun | GPU target |
| P2 | `LZ_RANS_V1` GPU | CPU reference plus kernel/end-to-end measurements | 8 MB/s until all size cells pass |

## Deferred Inputs

OASum remains unmaterialized. Its 1,065,019,104-byte `test.jsonl` is under
CC-BY-SA-3.0, so downloading it and publishing data-derived artifacts require
a separate owner approval. It is Tencent text/records evidence, not media or
executable routing evidence.

## Source Boundary

`E:/MIXER/KU/zstd-v1.5.7/` contains the verified BSD-3-Clause donor artifact.
The current build uses existing vendored zstd 1.6.0. Mode-43 evidence is
therefore labelled 1.6.0 until a separate production import pins source files
to the warehouse donor.
