#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "r2/core/byte_view.h"

namespace hz::r2 {

struct MatchCandidate {
    std::uint64_t distance = 0;
    std::uint32_t length = 0;
    std::uint8_t next_byte = 0;
    std::uint32_t confidence = 0;
    std::uint32_t estimated_parse_cost = 0;
};

class IMatchService {
public:
    virtual ~IMatchService() = default;

    virtual const char* name() const noexcept = 0;
    virtual void reset(std::uint64_t seed) = 0;
    virtual std::vector<MatchCandidate> find(
        ByteView history,
        std::size_t position,
        std::size_t maximum_candidates) = 0;
};

}  // namespace hz::r2

