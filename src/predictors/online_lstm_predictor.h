#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "core/profile.h"
#include "predictors/predictor.h"

class Lstm;

namespace hz {

struct OnlineLstmConfig {
    std::size_t output_size = kAlphabet;
    std::size_t cells = 200;
    std::size_t layers = 2;
    std::size_t horizon = 100;
    double learning_rate = 0.03;
    double gradient_clip = 10.0;
};

class OnlineLstmPredictor final : public Predictor {
public:
    explicit OnlineLstmPredictor(const Profile& profile);
    explicit OnlineLstmPredictor(OnlineLstmConfig config);
    ~OnlineLstmPredictor() override;

    void reset(std::uint64_t seed) override;
    void predict(const ByteHistory& history, ProbVector& out) override;
    void update(std::uint8_t actual, const ByteHistory& history) override;

private:
    OnlineLstmConfig config_;
    std::unique_ptr<Lstm> model_;
    ProbVector cached_{};
    bool prediction_ready_ = false;
};

}  // namespace hz
