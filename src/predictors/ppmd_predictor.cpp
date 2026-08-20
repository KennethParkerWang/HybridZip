#include "predictors/ppmd_predictor.h"

#include <cstdint>
#include <stdexcept>

#include "core/profile.h"
#include "ppmd_core.h"

namespace hz {

PpmdPredictor::PpmdPredictor()
    : PpmdPredictor(make_profile_v1()) {}

PpmdPredictor::PpmdPredictor(const Profile& profile)
    : PpmdPredictor(profile.ppmd_order, profile.ppmd_memory_bytes) {}

PpmdPredictor::PpmdPredictor(const int max_order,
                             const std::size_t memory_bytes)
    : core_(std::make_unique<cmix::PpmdCore>(max_order, memory_bytes)) {}

PpmdPredictor::~PpmdPredictor() = default;

void PpmdPredictor::reset(const std::uint64_t seed) {
    (void)seed;
    core_->reset();
}

void PpmdPredictor::predict(const ByteHistory& history, ProbVector& out) {
    (void)history;
    cmix::PpmdCore::WeightArray weights{};
    core_->predict(weights);

    std::uint64_t total = 0;
    for (const std::uint32_t weight : weights) {
        total += weight;
    }
    if (total == 0) {
        throw std::runtime_error("PPMd produced an empty probability distribution");
    }

    const double scale = 1.0 / static_cast<double>(total);
    for (std::size_t symbol = 0; symbol < weights.size(); ++symbol) {
        out[symbol] = static_cast<double>(weights[symbol]) * scale;
    }
}

void PpmdPredictor::update(const std::uint8_t actual,
                           const ByteHistory& history) {
    (void)history;
    core_->observe(actual);
}

std::size_t PpmdPredictor::current_context_depth() const noexcept {
    return core_->context_depth();
}

}  // namespace hz
