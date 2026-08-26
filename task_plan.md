# HybridZip R2 Continuation Plan

## Goal

Continue the donor-first R2-A through R2-D implementation until the current
portfolio is a runnable, byte-exact HybridZip product with HZ01 compatibility,
decoder-visible routing, and a final archive-byte experiment ledger.

## Phases

- [x] Phase 1: Record current state and preserve the existing R2 plan.
- [x] Phase 2: Audit the next R2-D donor candidates in `E:/MIXER/KU`.
- [x] Phase 3: Port one license-cleared, runnable candidate into HZ02.
- [ ] Phase 4: Release-build and run the single allowed 1 KiB round-trip gate.
- [x] Phase 5: Update provenance, format, ledger, and choose the next candidate.

## Constraints

- Preserve HZ01 and PROFILE_V1 behavior.
- Prefer existing donor code; do not rewrite a mature codec from scratch.
- Before final implementation is declared complete, gate each new branch with
  one deterministic 1 KiB encode/decode smoke only.
- Do not rerun Auto, D40, CTest, batch, or larger blocks without explicit user
  approval.
- Keep downloaded material and provenance under `E:/MIXER/KU`.

## Status

Phase 2 audit found no additional model-free donor with a complete runnable
C++17 closure. The current warehouse donors are already represented as
`ported` or explicitly limited by unavailable checkpoints. The existing
portfolio is therefore the candidate set for the next accounting pass. A
small Auto family-gating defect was fixed in `src/r2/block/block_planner.cpp`;
mode 41 is now gated by the neural family and mode 42 by the numeric family.
Release incremental compilation passed on 2026-08-26. Phase 4 remains pending
because the current user constraint forbids a new Auto/CTest run.

Read-only evidence audit on 2026-08-26 found 50 smoke directories whose names
contain `1k` and 59 `verification.json` files under `results/smoke`. These
include historical and rebuild duplicates, so they are evidence inventory only
and are not counted as a 43-mode final ledger. No new runtime test was started.

Post-build gate update: the current Release binary completed forced 1 KiB
encode/decode smokes for mode 41 (`lmic-arithmetic`) and mode 42
(`delta-binary-packed-zstd`). Both produced 1024 decoded bytes with exact
input/output SHA-256 equality. Phase 4 remains open for the final portfolio
because these checks are branch gates, not Auto or corpus evidence.

Added and ran `tools/index_r2_smoke_evidence.ps1`; after filtering to the
current Release hash, it reports unique 1 KiB evidence for 2/43 modes. The
remaining records are historical-hash evidence and require refreshed gates or
the final authorized ledger run.

An unfiltered metadata scan found 14 qualifying records for 11/43 explicit
mode tags (23 and 30..42). Router-only records are intentionally not inferred
as selected-mode evidence. This is now the preflight baseline for any future
one-mode-at-a-time smoke authorization.

Donor warehouse validation passed with 2506 checks across 21 manifests and
their provenance/license evidence. No additional complete model-free C++17
decoder closure was found, so implementation proceeds with the existing
portfolio while the final ledger remains pending.

The next one-mode gate completed for mode 40 (`kanzi-ans`) on the current
Release binary. The current-hash evidence index now covers 3/43 modes (40..42)
with byte-exact 1 KiB records.

Execution policy update on 2026-08-26: the user authorized three-way parallel
execution for the remaining current-Release 1 KiB forced-mode gates. Each mode
must retain a unique output directory and separate stdout/stderr; index and
documentation refresh remain serialized after the workers finish.

Mode 39 (`lz4`) then passed the same one-mode 1 KiB gate. Current-hash
coverage is now 4/43 modes (39..42).

Mode 38 (`wavpack`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1289 bytes (10.070312 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
5/43 modes (38..42). Two earlier harness attempts failed before codec launch
and produced no accepted codec evidence.

Mode 37 (`paq8px-detected-sse`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1110 bytes (8.671875 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 6/43 modes (37..42).

Mode 36 (`paq8px-generic-sse`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1088 bytes (8.5 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 7/43 modes (36..42).

Mode 35 (`paq8px-similarity-sse`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1086 bytes (8.484375 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 8/43 modes (35..42).

Mode 34 (`paq8px-similarity`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1084 bytes (8.46875 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 9/43 modes (34..42).

Mode 33 (`paq8px-linear-prediction`) then passed the same one-mode 1 KiB gate
on the current Release binary. The archive was 1106 bytes (8.640625 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 10/43 modes (33..42).

Mode 32 (`paq8px-record-model`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1084 bytes (8.46875 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 11/43 modes (32..42).

Mode 31 (`fastpfor`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1106 bytes (8.640625 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
12/43 modes (31..42).

Mode 30 (`ctw`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1135 bytes (8.867188 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
13/43 modes (30..42).

Mode 29 (`zpaq`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1365 bytes (10.664063 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
14/43 modes (29..42). Evidence is in
`results/smoke/r2-postbuild-zpaq-1k-20260826/verification.json`; the refreshed
index is `results/analysis/r2-smoke-evidence-index-20260826-mode29`.

Mode 28 (`ppmd8`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1194 bytes (9.328125 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
15/43 modes (28..42). Evidence is in
`results/smoke/r2-postbuild-ppmd8-1k-20260826/verification.json`; the refreshed
index is `results/analysis/r2-smoke-evidence-index-20260826-mode28`.

Mode 27 (`ppmd7`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1193 bytes (9.320313 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
16/43 modes (27..42). Evidence is in
`results/smoke/r2-postbuild-ppmd7-1k-20260826/verification.json`; the refreshed
index is `results/analysis/r2-smoke-evidence-index-20260826-mode27`.

Mode 26 (`jax-compress-portable`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1141 bytes (8.914063 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 17/43 modes (26..42). Evidence is in
`results/smoke/r2-postbuild-jax-compress-portable-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode26`.

Mode 25 (`bgpt-shared-prior`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1759 bytes (13.742188 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 18/43 modes (25..42). Evidence is in
`results/smoke/r2-postbuild-bgpt-shared-prior-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode25`.

Mode 24 (`delta-of-delta-zstd`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1099 bytes (8.585938 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 19/43 modes (24..42). Evidence is in
`results/smoke/r2-postbuild-delta-of-delta-zstd-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode24`.

Mode 23 (`lstm-compress`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1098 bytes (8.578125 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 20/43 modes (23..42). Evidence is in
`results/smoke/r2-postbuild-lstm-compress-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode23`.

Mode 22 (`shared-neural-lstm`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1099 bytes (8.585938 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 21/43 modes (22..42). Evidence is in
`results/smoke/r2-postbuild-shared-neural-lstm-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode22`.

Mode 21 (`neural-lstm`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1094 bytes (8.546875 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
22/43 modes (21..42). Evidence is in
`results/smoke/r2-postbuild-neural-lstm-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode21`.

Mode 20 (`cmix-word-zstd`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1403 bytes (10.960938 bpb), decoded
to 1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 23/43 modes (20..42). Evidence is in
`results/smoke/r2-postbuild-cmix-word-zstd-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode20`.

Mode 19 (`brotli-text`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1088 bytes (8.5 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 24/43 modes (19..42). Evidence is in
`results/smoke/r2-postbuild-brotli-text-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode19`.

Mode 18 (`flac-residual`) then passed the same one-mode 1 KiB gate on the
current Release binary. The archive was 1182 bytes (9.234375 bpb), decoded to
1024 bytes, and matched the input SHA-256 exactly. Current-hash coverage is
now 25/43 modes (18..42). Evidence is in
`results/smoke/r2-postbuild-flac-residual-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode18`.

Mode 17 (`jpegls`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1333 bytes (10.414063 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
26/43 modes (17..42). Evidence is in
`results/smoke/r2-postbuild-jpegls-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode17`.

Mode 16 (`record-transpose-zstd`) then passed the same one-mode 1 KiB gate on
the current Release binary. The archive was 1099 bytes (8.585938 bpb),
decoded to 1024 bytes, and matched the input SHA-256 exactly. Current-hash
coverage is now 27/43 modes (16..42). Evidence is in
`results/smoke/r2-postbuild-record-transpose-zstd-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode16`.

Mode 15 (`bcj2-zstd`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1119 bytes (8.742188 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
28/43 modes (15..42). Evidence is in
`results/smoke/r2-postbuild-bcj2-zstd-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode15`.

Mode 14 (`rans`) then passed the same one-mode 1 KiB gate on the current
Release binary. The archive was 1828 bytes (14.28125 bpb), decoded to 1024
bytes, and matched the input SHA-256 exactly. Current-hash coverage is now
29/43 modes (14..42). Evidence is in
`results/smoke/r2-postbuild-rans-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode14`.

Mode 13 (`fastpfor`) then passed the same one-mode 1 KiB gate on the current
Release binary. The original output name was already occupied by a different
historical mode-31 record, so the accepted evidence uses a unique mode-tagged
directory. The archive was 1106 bytes (8.640625 bpb), decoded to 1024 bytes,
and matched the input SHA-256 exactly. Current-hash coverage is now 30/43
modes (13..42). Evidence is in
`results/smoke/r2-postbuild-fastpfor-mode13-1k-20260826/verification.json`;
the refreshed index is `results/analysis/r2-smoke-evidence-index-20260826-mode13`.

The user then authorized three-way parallel execution for the remaining
current-Release 1 KiB gates. The reusable worker is
`tools/run_r2_postbuild_parallel_1k.ps1`; it uses unique per-mode directories,
separate stdout/stderr, a 60-second per-process timeout, and skips accepted
current-hash evidence. Modes 11, 10, 9, 7, 6, 5, 4, 3, 2, 1, and 0 passed
byte-exact encode/decode. Mode 8 (`bwt-rlt-zstd`) was intentionally recorded as
not applicable to the random 1 KiB gate because Kanzi RLT did not reduce the
BWT block; its failure record is
`results/smoke/r2-postbuild-bwt-rlt-zstd-mode8-1k-20260826-230041-parallel/failure.json`.
The current-hash index is
`results/analysis/r2-smoke-evidence-index-20260826-parallel` and reports
42/43 unique passing modes with only mode 8 missing. No Auto, D40, CTest, or
larger-block test was run.
