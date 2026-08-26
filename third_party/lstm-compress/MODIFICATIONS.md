# lstm-compress HybridZip Modifications

The donor checkout under `E:/MIXER/KU/hybridzip-r2/neural/online/lstm-compress`
is retained as the authoritative source. HybridZip does not compile the donor
application runner directly.

- Converted the reusable predictor, LSTM, byte model, and range coder closure
  to `port/lstm_compress_donor_port.{h,cpp}` with the `hz::r2` namespace and
  memory-based APIs.
- Replaced global C stream/file framing with `ByteView` input and vector output.
- Replaced the donor process-global `rand()` use with a private deterministic
  MSVC-compatible recurrence seeded per model instance; this avoids cross-mode
  state leakage while retaining the Windows donor sequence.
- Preserved the donor 256-slot byte interval for used-symbol bitmap profiles;
  inactive slots are zeroed only after a complete observed byte, matching the
  donor `ByteModel` lifecycle.
- Added `HLC1` versioned framing, decoder-visible vocabulary metadata, expected
  size checks, and a bounded four-byte EOF compatibility pad.
- Kept optional dictionary preprocessing outside this first coding path; it is a
  separate representation candidate and is not silently claimed as ported.

The mode-23 backend keeps the old untagged decoder for archives produced before
the donor port. New payloads ignore the HZ02 `model_seed` because the donor
profile has a fixed, versioned initialization sequence.
