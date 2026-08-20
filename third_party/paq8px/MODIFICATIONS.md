# PAQ8px MatchCore modification notice

Modified for HybridZip on 2026-08-20 from PAQ8px revision
`29237fb44cb1995690e3eb72c6c3b1e4aede5791`.

The adapted work remains available under the upstream GNU GPL version 2 or
later terms. License evidence and attribution are in `LICENSE.md`; exact donor
files and hashes are in `PROVENANCE.md`.

## Extracted behavior

- Replaced PAQ8px `Array`, `Shared`, and ring-buffer dependencies with C++17
  fixed-width types, `std::array`, and an exactly sized owned hash table.
- Preserved one shared table for the order-9, order-7, and order-5 hashes,
  queried in that order. Each bucket retains the three most recent positions.
- Preserved exact context verification, four active candidates, donor-index
  deduplication, registration strengths `9 -> 5`, `7 -> 3`, `5 -> 1`, and
  `MatchInfo::prio()` ordering.
- Preserved strength saturation at 65,535 and the recovery stability threshold
  of three matching bytes.
- Added a separate `contiguous_length`: it starts at the verified context
  length, increments on ordinary extension, becomes zero on mismatch, and
  restarts at one on the first recovered byte. PAQ strength continues from its
  backed-up value, as in the donor.
- Retained only PAQ8px's generic mode. The generic `NormalModel` recurrence is
  `(previous_order_hash + byte * 30 + order) * PHI64`, where 30 is the pinned
  donor's `BlockType::Count` and `DEFAULT` is zero.
- Guarded hash bits to 1 through 26. The upper limit matches PAQ8px's largest
  configured match table and prevents configurations beyond the donor's
  practical resource range. The default 20-bit table contains `2^20` buckets
  of 12 bytes, exactly 12,582,912 allocated bytes.
- Rejects positions beyond the 32-bit block-relative index used by the donor.

## Bit-to-byte recovery conversion

PAQ8px calls `MatchInfo::update()` before every predicted bit. On the first
wrong bit of a byte, it saves `length` and `index`, clears `length`, and sets
`delta` so the remaining bits of that same byte can use a delta context. At
the following `bpos == 0`, it clears `delta`; the candidate is then in
pre-recovery and predicts nothing for one complete byte. At the next byte
boundary, that skipped byte is compared with the donor byte one position past
the mismatch. Equality resumes recovery; inequality drops the candidate. A
second mismatch during recovery drops the candidate immediately.

HybridZip's service is called only before whole bytes, so there is no API
boundary inside the mismatching byte where delta predictions could be
returned. `MatchCore` performs the same delta transition internally while
atomically consuming that byte, and the next externally visible state is
pre-recovery. It then suppresses exactly one byte and applies the same donor
index/strength increments. Recovery remains visible until three contiguous
bytes have matched after the gap.

## HybridZip adapters

The project-owned adapters in `src/r2/match/paq8px_match_service.{h,cpp}` and
`src/r2/experts/paq8px_match_expert.{h,cpp}` add monotonic history traversal,
same-position caching, `IMatchService` field conversion, and an `IExpert`
predict-before-observe lifecycle. Match evidence is advisory: parse cost is
set to `UINT32_MAX`, so it is never treated as an LZ phrase.

`tests/paq8px_match_tests.cpp` covers the donor-golden lookup order, candidate
lifecycle, recovery, saturation, allocation guards, reset, cache behavior,
and both project-owned adapters.

`src/r2/entropy/donor_match_predictive_backend.{h,cpp}` consumes this Match
evidence alongside the separately governed cmix Match posterior. Its
project-owned probability fusion and encode/decode lifecycle are covered by
`tests/donor_match_predictive_backend_tests.cpp`.
