#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"

namespace hz::r2 {

enum class EntropyKind : std::uint8_t {
    Stored = 0,
    SymbolArithmetic = 1,
    ZstdFse = 2,
    Rans = 3,
    Fse = 4,
    Lzma = 5,
    FastPfor = 6,
    JpegLs = 7,
    FlacResidual = 8,
    BrotliText = 9,
    Ppmd7 = 10,
    Ppmd8 = 11,
    Zpaq = 12,
    Ctw = 13,
    Paq8pxApm = 14,
    Paq8pxRecordModel = 15,
    Paq8pxLinearPrediction = 16,
    Paq8pxSimilarity = 17,
    Paq8pxSimilaritySse = 18,
    Paq8pxGenericSse = 19,
    Paq8pxDetectedSse = 20,
    Wavpack = 21,
    Lz4 = 22,
    KanziAns = 23,
    LmicArithmetic = 24,
    ZstdFseDeltaBinaryPacked = 25
};

class IBlockEntropyBackend {
public:
    virtual ~IBlockEntropyBackend() = default;

    virtual const char* name() const noexcept = 0;
    virtual EntropyKind kind() const noexcept = 0;
    virtual std::vector<std::uint8_t> encode(ByteView input) const = 0;
    virtual std::vector<std::uint8_t> decode(
        ByteView payload,
        std::size_t expected_size) const = 0;
};

}  // namespace hz::r2
