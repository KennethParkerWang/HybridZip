#pragma once

#include <cstdint>
#include <vector>

namespace hz::r2::paq8px {

// Adapted from paq8px src/APM1.{hpp,cpp}.  The table layout, stretch/squash
// interpolation, and update rule remain donor-derived; Shared and the
// donor's broadcaster are intentionally replaced by an explicit bit update.
class Apm1 final {
public:
    Apm1(std::uint32_t context_count, std::uint32_t rate);

    std::uint32_t predict(std::uint32_t probability,
                          std::uint32_t context);
    void update(std::uint8_t bit);

private:
    static int squash(int distance);
    static int stretch(std::uint32_t probability);

    std::uint32_t index_ = 0;
    std::uint32_t context_count_ = 0;
    std::uint32_t rate_ = 0;
    std::vector<std::uint16_t> table_;
};

}  // namespace hz::r2::paq8px
