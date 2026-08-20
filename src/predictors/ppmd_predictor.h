#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "predictors/predictor.h"

namespace hz {

struct Profile;
namespace cmix {
class PpmdCore;
}

class PpmdPredictor final : public Predictor {
public:
    PpmdPredictor();
    explicit PpmdPredictor(const Profile& profile);
    PpmdPredictor(int max_order, std::size_t memory_bytes);
    ~PpmdPredictor() override;

    PpmdPredictor(const PpmdPredictor&) = delete;
    PpmdPredictor& operator=(const PpmdPredictor&) = delete;

    void reset(std::uint64_t seed) override;
    void predict(const ByteHistory& history, ProbVector& out) override;
    void update(std::uint8_t actual, const ByteHistory& history) override;

    std::size_t current_context_depth() const noexcept;

private:
    std::unique_ptr<cmix::PpmdCore> core_;
};

}  // namespace hz
