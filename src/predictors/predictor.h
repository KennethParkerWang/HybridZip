#pragma once

#include <cstdint>

#include "core/types.h"

namespace hz {

class ByteHistory;

class Predictor {
public:
    virtual ~Predictor() = default;

    virtual void reset(std::uint64_t seed) = 0;
    virtual void predict(const ByteHistory& history, ProbVector& out) = 0;
    virtual void update(std::uint8_t actual, const ByteHistory& history) = 0;
};

}  // namespace hz
