#pragma once

#include <cstddef>
#include <cstdint>

namespace hz::r2 {

constexpr std::size_t kBgptSharedPriorContextCount = 257U;
constexpr std::size_t kBgptSharedPriorStartContext = 256U;
constexpr std::uint32_t kBgptSharedPriorFrequencyTotal = 1U << 16U;

const std::uint16_t* bgpt_shared_prior_frequencies() noexcept;

}  // namespace hz::r2
