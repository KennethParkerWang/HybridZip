# LMIC port boundary

- Converted the donor base-2, precision-32 arithmetic-coder state machine to
  C++17 in `src/r2/entropy/lmic_arithmetic_backend.cpp`.
- Retained donor inclusive `[low, high]` interval arithmetic, matching-digit
  normalization, carry-digit normalization, termination, and decoder padding.
- Replaced the unavailable JAX/Haiku Transformer posterior with the existing
  decoder-synchronised frozen bGPT byte prior. This is recorded in the mode and
  payload identity as `lmic-arithmetic-frozen-bgpt-v1`.
- Added an `HLM1` payload envelope containing framing version, left-padding bit
  count, base, and precision. Existing HZ01 and all earlier HZ02 modes are
  unchanged.
- No donor Python source was modified.
