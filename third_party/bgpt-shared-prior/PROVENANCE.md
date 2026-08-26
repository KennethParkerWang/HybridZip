# bGPT Shared-Prior Projection

- Source: `https://github.com/sanderwood/bgpt`
- Source revision: `56b98d647b97c086bf9b3c0b840f0d662545e81c`
- Checkpoint: `https://huggingface.co/sander-wood/bgpt`
- Checkpoint revision: `7b3fc8b7fe0b4fec4f40dd0bdeb39b2cacf0aa96`
- Checkpoint file: `weights-text.pth`
- Checkpoint bytes: `1324402756`
- Checkpoint SHA-256:
  `F30ED5A814086C5B9E64F56A76CCFCDED00A82FC71C3BA6DE322B708D29F6AC7`
- License: MIT; retained in `LICENSE`.
- Download date: 2026-08-21.
- Warehouse source: `E:/MIXER/KU/hybridzip-r2/neural/shared/bgpt`
- Warehouse checkpoint:
  `E:/MIXER/KU/hybridzip-r2/neural/shared/bgpt-weights/weights-text.pth`

`tools/extract_bgpt_shared_prior.py` reads the pinned PyTorch ZIP checkpoint
without PyTorch, reproduces the donor GPT-2 forward equations with NumPy, and
generates `src/r2/entropy/bgpt_shared_prior_data.cpp`. The projection runs the
patch decoder on a fixed all-special-token bootstrap and retains the byte
decoder's start and previous-byte posteriors. It is a bounded 257-context
shared prior, not the complete 110M bGPT runtime.

The generated uint16 frequency table SHA-256 is
`1B135959D42B304D0BC4CCE9DA75022D4A8BE7BC30061C5FE67405F1B2D06330`.
Every row sums to 65536 and is expanded deterministically to CDF24 by
multiplying each frequency by 256.
