#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace hz {

constexpr std::size_t kAlphabet = 256;
constexpr std::uint32_t kCdfBits = 24;
constexpr std::uint32_t kCdfTotal = 1U << kCdfBits;
constexpr int kCoderStateBits = 32;
constexpr double kProbFloor = 1e-12;
constexpr std::uint64_t kDefaultModelSeed = 0x485A5F56315F3031ULL;

using ProbVector = std::array<double, kAlphabet>;

struct Cdf {
    std::array<std::uint32_t, kAlphabet + 1> v{};
};

}  // namespace hz
