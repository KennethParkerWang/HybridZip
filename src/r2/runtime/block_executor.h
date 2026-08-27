#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "r2/block/block_planner.h"

namespace hz::r2 {

struct FastBlockTask {
    std::uint32_t index = 0;
    std::uint32_t checksum = 0;
    std::vector<std::uint8_t> raw;
};

struct FastBlockResult {
    std::uint32_t index = 0;
    std::uint32_t checksum = 0;
    std::uint32_t uncompressed_size = 0;
    BlockDecision decision;
};

// Fast planners have no cross-block state, so completed blocks can be
// reordered here and serialized by the caller in their original order.
class FastBlockExecutor final {
public:
    FastBlockExecutor(BlockPlannerOptions planner_options,
                      std::uint32_t worker_count);
    ~FastBlockExecutor();

    FastBlockExecutor(const FastBlockExecutor&) = delete;
    FastBlockExecutor& operator=(const FastBlockExecutor&) = delete;

    void submit(FastBlockTask task);
    FastBlockResult take(std::uint32_t expected_index);

private:
    struct State;
    std::unique_ptr<State> state_;
};

}  // namespace hz::r2
