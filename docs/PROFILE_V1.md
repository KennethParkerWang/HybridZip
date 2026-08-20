# PROFILE_V1

PROFILE_V1 is identified by `profile_id = 1`. Its parameters are part of the
archive contract and are not command-line options.

## Common

- Alphabet: bytes `0..255`
- ByteHistory capacity: 8 MiB
- Model seed: `0x485A5F56315F3031`
- Predictor order: NGram, PPMD, Match, OnlineLSTM

## NGram

- Orders: 0 through 4
- Online sparse continuation counts
- Interpolation: `lambda(n) = n / (n + 32)`
- Memory target: approximately 64 MiB
- At capacity, existing entries continue updating and new entries are not made

## PPMD

- Maximum order: 12
- Model memory: 64 MiB
- All 256 byte symbols enabled
- Prediction query and observed-byte update are separate operations

## Match

- Context: last 8 bytes
- Hash slots: `2^20`
- History window: 8 MiB
- Minimum verified match: 3 bytes
- Confidence buckets: 256
- Candidate probability: `(hits + 1) / (trials + 2)`

## Online LSTM

- External auxiliary features: 0
- Input: previous byte as a 256-way one-hot symbol
- Output: 256-way softmax
- Cells per layer: 200
- Layers: 2
- BPTT horizon: 100
- Learning rate: 0.03
- Gradient clip: 10
- Initialization: deterministic SplitMix64 stream derived from the model seed
- State and optimizer continue for the full file; there is no block reset

The current LSTM uses floating-point cmix computations. Encoder and decoder are
deterministic within the released binary and tested toolchain. Cross-compiler,
cross-ISA bitstream identity remains a later validation item.

## Mixer

- Four equal initial weights: 0.25 each
- Learning rate `eta = 0.5`
- Log-weight update uses the observed symbol probability with floor `1e-12`

## CDF And Coder

- CDF total: `2^24`
- One count reserved for every symbol
- Remaining counts: largest-remainder allocation
- Fraction ties: lower symbol ID first
- Arithmetic coder state: 32 bits
