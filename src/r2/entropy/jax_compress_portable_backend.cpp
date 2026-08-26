#include "r2/entropy/jax_compress_portable_backend.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/cdf.h"
#include "core/probability.h"
#include "core/types.h"
#include "entropy/arithmetic_codec.h"

namespace hz::r2 {
namespace {

constexpr std::size_t kSequenceLength = 8U;
constexpr std::size_t kEmbeddingSize = 8U;
constexpr std::size_t kUnits = 16U;
constexpr std::size_t kLayers = 2U;
constexpr std::size_t kGateCount = 4U;
constexpr std::size_t kDenseInput = kLayers * kUnits;
constexpr std::size_t kRetrainPeriod = 4096U;
constexpr std::size_t kRetrainBlock = 256U;
constexpr std::size_t kRetrainStride = 8U;
constexpr double kLearningRate = 0.0005;
constexpr double kAdamBeta1 = 0.0;
constexpr double kAdamBeta2 = 0.9999;
constexpr double kAdamEpsilon = 1e-12;
constexpr double kGradientClip = 4.0;
constexpr std::uint64_t kInitializationSeed = 1234U;

struct ParameterVector {
    std::vector<float> value;
    std::vector<double> first_moment;
    std::vector<double> second_moment;
    std::vector<double> gradient;

    explicit ParameterVector(const std::size_t size = 0U)
        : value(size), first_moment(size), second_moment(size),
          gradient(size) {}

    void clear_gradient() {
        std::fill(gradient.begin(), gradient.end(), 0.0);
    }
};

struct LayerParameters {
    std::size_t input_size = 0U;
    ParameterVector weights;
    ParameterVector bias;

    explicit LayerParameters(const std::size_t input = 0U)
        : input_size(input),
          weights(kGateCount * kUnits * (input + kUnits)),
          bias(kGateCount * kUnits) {}
};

struct ModelState {
    std::array<std::vector<double>, kLayers> cell;
    std::array<std::vector<double>, kLayers> hidden;

    ModelState() {
        for (std::size_t layer = 0; layer < kLayers; ++layer) {
            cell[layer].assign(kUnits, 0.0);
            hidden[layer].assign(kUnits, 0.0);
        }
    }
};

struct StepCache {
    std::uint8_t input_symbol = 0U;
    std::vector<double> input;
    std::vector<double> previous_cell;
    std::vector<double> previous_hidden;
    std::vector<double> input_gate;
    std::vector<double> forget_gate;
    std::vector<double> candidate;
    std::vector<double> output_gate;
    std::vector<double> cell;
    std::vector<double> hidden;
};

struct ForwardResult {
    ModelState final_state;
    std::array<std::array<StepCache, kSequenceLength>, kLayers> cache;
    std::vector<double> dense_input;
    hz::ProbVector probability{};
};

std::uint64_t splitmix64(std::uint64_t& state) noexcept {
    state += 0x9E3779B97F4A7C15ULL;
    std::uint64_t value = state;
    value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31U);
}

double uniform_signed(std::uint64_t& state) noexcept {
    const double unit = static_cast<double>(splitmix64(state) >> 11U) *
        (1.0 / 9007199254740992.0);
    return unit * 2.0 - 1.0;
}

double sigmoid(const double value) noexcept {
    const double clamped = std::max(-30.0, std::min(30.0, value));
    return 1.0 / (1.0 + std::exp(-clamped));
}

std::vector<std::uint8_t> as_bytes(const std::string& value) {
    return {reinterpret_cast<const std::uint8_t*>(value.data()),
            reinterpret_cast<const std::uint8_t*>(value.data()) + value.size()};
}

class PortableOnlineModel {
public:
    PortableOnlineModel()
        : embedding_(hz::kAlphabet * kEmbeddingSize),
          layers_{LayerParameters(kEmbeddingSize),
                  LayerParameters(kEmbeddingSize + kUnits)},
          dense_weights_(hz::kAlphabet * kDenseInput),
          dense_bias_(hz::kAlphabet) {
        std::uint64_t random_state = kInitializationSeed;
        initialize(embedding_, random_state,
                   std::sqrt(6.0 / (hz::kAlphabet + kEmbeddingSize)));
        for (std::size_t layer = 0; layer < kLayers; ++layer) {
            const double limit = std::sqrt(6.0 /
                (layers_[layer].input_size + kUnits + kGateCount * kUnits));
            initialize(layers_[layer].weights, random_state, limit);
            for (std::size_t unit = 0; unit < kUnits; ++unit) {
                layers_[layer].bias.value[kUnits + unit] = 1.0F;
            }
        }
        initialize(dense_weights_, random_state,
                   std::sqrt(6.0 / (kDenseInput + hz::kAlphabet)));
    }

    ForwardResult forward(
        const std::array<std::uint8_t, kSequenceLength>& sequence,
        const ModelState& initial_state) const {
        ForwardResult result{};
        ModelState state = initial_state;
        for (std::size_t time = 0; time < kSequenceLength; ++time) {
            std::vector<double> embedding(kEmbeddingSize);
            const std::size_t embedding_offset =
                static_cast<std::size_t>(sequence[time]) * kEmbeddingSize;
            for (std::size_t index = 0; index < kEmbeddingSize; ++index) {
                embedding[index] = embedding_.value[embedding_offset + index];
            }

            std::vector<double> previous_layer_hidden;
            for (std::size_t layer = 0; layer < kLayers; ++layer) {
                StepCache& cache = result.cache[layer][time];
                cache.input_symbol = sequence[time];
                cache.input = embedding;
                if (layer != 0U) {
                    cache.input.insert(cache.input.end(),
                                       previous_layer_hidden.begin(),
                                       previous_layer_hidden.end());
                }
                cache.previous_cell = state.cell[layer];
                cache.previous_hidden = state.hidden[layer];
                cache.input_gate.assign(kUnits, 0.0);
                cache.forget_gate.assign(kUnits, 0.0);
                cache.candidate.assign(kUnits, 0.0);
                cache.output_gate.assign(kUnits, 0.0);
                cache.cell.assign(kUnits, 0.0);
                cache.hidden.assign(kUnits, 0.0);

                const LayerParameters& parameters = layers_[layer];
                const std::size_t width = parameters.input_size + kUnits;
                for (std::size_t unit = 0; unit < kUnits; ++unit) {
                    std::array<double, kGateCount> gate{};
                    for (std::size_t kind = 0; kind < kGateCount; ++kind) {
                        const std::size_t row = kind * kUnits + unit;
                        double total = parameters.bias.value[row];
                        for (std::size_t index = 0;
                             index < parameters.input_size; ++index) {
                            total += static_cast<double>(
                                parameters.weights.value[row * width + index]) *
                                cache.input[index];
                        }
                        for (std::size_t index = 0; index < kUnits; ++index) {
                            total += static_cast<double>(parameters.weights.value[
                                row * width + parameters.input_size + index]) *
                                cache.previous_hidden[index];
                        }
                        gate[kind] = total;
                    }
                    cache.input_gate[unit] = sigmoid(gate[0]);
                    cache.forget_gate[unit] = sigmoid(gate[1]);
                    cache.candidate[unit] = std::tanh(gate[2]);
                    cache.output_gate[unit] = sigmoid(gate[3]);
                    cache.cell[unit] = cache.forget_gate[unit] *
                            cache.previous_cell[unit] +
                        cache.input_gate[unit] * cache.candidate[unit];
                    cache.hidden[unit] = cache.output_gate[unit] *
                        std::tanh(cache.cell[unit]);
                }
                state.cell[layer] = cache.cell;
                state.hidden[layer] = cache.hidden;
                previous_layer_hidden = cache.hidden;
            }
        }
        result.final_state = state;
        result.dense_input.reserve(kDenseInput);
        for (std::size_t layer = 0; layer < kLayers; ++layer) {
            result.dense_input.insert(result.dense_input.end(),
                                      state.hidden[layer].begin(),
                                      state.hidden[layer].end());
        }

        std::array<double, hz::kAlphabet> logits{};
        double maximum = -std::numeric_limits<double>::infinity();
        for (std::size_t symbol = 0; symbol < hz::kAlphabet; ++symbol) {
            double total = dense_bias_.value[symbol];
            for (std::size_t index = 0; index < kDenseInput; ++index) {
                total += static_cast<double>(
                    dense_weights_.value[symbol * kDenseInput + index]) *
                    result.dense_input[index];
            }
            logits[symbol] = total;
            maximum = std::max(maximum, total);
        }
        double normalizer = 0.0;
        for (std::size_t symbol = 0; symbol < hz::kAlphabet; ++symbol) {
            result.probability[symbol] = std::exp(logits[symbol] - maximum);
            normalizer += result.probability[symbol];
        }
        if (!std::isfinite(normalizer) || normalizer <= 0.0) {
            hz::set_uniform_probability(result.probability);
        } else {
            for (double& probability : result.probability) {
                probability /= normalizer;
            }
        }
        return result;
    }

    void update(const ForwardResult& result, const std::uint8_t target) {
        clear_gradients();
        std::array<double, hz::kAlphabet> output_gradient{};
        for (std::size_t symbol = 0; symbol < hz::kAlphabet; ++symbol) {
            output_gradient[symbol] = result.probability[symbol] -
                (symbol == target ? 1.0 : 0.0);
            dense_bias_.gradient[symbol] += output_gradient[symbol];
            for (std::size_t index = 0; index < kDenseInput; ++index) {
                dense_weights_.gradient[symbol * kDenseInput + index] +=
                    output_gradient[symbol] * result.dense_input[index];
            }
        }

        using TimeGradient =
            std::array<std::array<std::vector<double>, kSequenceLength>, kLayers>;
        TimeGradient hidden_gradient{};
        for (std::size_t layer = 0; layer < kLayers; ++layer) {
            for (std::size_t time = 0; time < kSequenceLength; ++time) {
                hidden_gradient[layer][time].assign(kUnits, 0.0);
            }
        }
        for (std::size_t layer = 0; layer < kLayers; ++layer) {
            for (std::size_t unit = 0; unit < kUnits; ++unit) {
                const std::size_t dense_index = layer * kUnits + unit;
                double gradient = 0.0;
                for (std::size_t symbol = 0; symbol < hz::kAlphabet; ++symbol) {
                    gradient += static_cast<double>(dense_weights_.value[
                        symbol * kDenseInput + dense_index]) *
                        output_gradient[symbol];
                }
                hidden_gradient[layer][kSequenceLength - 1U][unit] += gradient;
            }
        }

        std::array<std::vector<double>, kLayers> recurrent_hidden;
        std::array<std::vector<double>, kLayers> recurrent_cell;
        for (std::size_t layer = 0; layer < kLayers; ++layer) {
            recurrent_hidden[layer].assign(kUnits, 0.0);
            recurrent_cell[layer].assign(kUnits, 0.0);
        }

        for (std::size_t reverse_time = 0;
             reverse_time < kSequenceLength; ++reverse_time) {
            const std::size_t time = kSequenceLength - 1U - reverse_time;
            std::vector<double> embedding_gradient(kEmbeddingSize, 0.0);
            for (std::size_t reverse_layer = 0;
                 reverse_layer < kLayers; ++reverse_layer) {
                const std::size_t layer = kLayers - 1U - reverse_layer;
                const StepCache& cache = result.cache[layer][time];
                LayerParameters& parameters = layers_[layer];
                const std::size_t width = parameters.input_size + kUnits;
                std::vector<double> gate_gradient(kGateCount * kUnits, 0.0);
                std::vector<double> previous_cell_gradient(kUnits, 0.0);

                for (std::size_t unit = 0; unit < kUnits; ++unit) {
                    const double hidden_total =
                        hidden_gradient[layer][time][unit] +
                        recurrent_hidden[layer][unit];
                    const double tanh_cell = std::tanh(cache.cell[unit]);
                    const double output_delta = hidden_total * tanh_cell;
                    const double cell_total = recurrent_cell[layer][unit] +
                        hidden_total * cache.output_gate[unit] *
                            (1.0 - tanh_cell * tanh_cell);
                    const double forget_delta =
                        cell_total * cache.previous_cell[unit];
                    const double input_delta =
                        cell_total * cache.candidate[unit];
                    const double candidate_delta =
                        cell_total * cache.input_gate[unit];
                    previous_cell_gradient[unit] =
                        cell_total * cache.forget_gate[unit];
                    gate_gradient[unit] = input_delta *
                        cache.input_gate[unit] *
                        (1.0 - cache.input_gate[unit]);
                    gate_gradient[kUnits + unit] = forget_delta *
                        cache.forget_gate[unit] *
                        (1.0 - cache.forget_gate[unit]);
                    gate_gradient[2U * kUnits + unit] = candidate_delta *
                        (1.0 - cache.candidate[unit] * cache.candidate[unit]);
                    gate_gradient[3U * kUnits + unit] = output_delta *
                        cache.output_gate[unit] *
                        (1.0 - cache.output_gate[unit]);
                }

                std::vector<double> concatenated_gradient(width, 0.0);
                for (std::size_t row = 0; row < gate_gradient.size(); ++row) {
                    const double gradient = gate_gradient[row];
                    parameters.bias.gradient[row] += gradient;
                    for (std::size_t index = 0;
                         index < parameters.input_size; ++index) {
                        parameters.weights.gradient[row * width + index] +=
                            gradient * cache.input[index];
                        concatenated_gradient[index] +=
                            static_cast<double>(parameters.weights.value[
                                row * width + index]) * gradient;
                    }
                    for (std::size_t index = 0; index < kUnits; ++index) {
                        const std::size_t offset = parameters.input_size + index;
                        parameters.weights.gradient[row * width + offset] +=
                            gradient * cache.previous_hidden[index];
                        concatenated_gradient[offset] +=
                            static_cast<double>(parameters.weights.value[
                                row * width + offset]) * gradient;
                    }
                }

                for (std::size_t index = 0; index < kEmbeddingSize; ++index) {
                    embedding_gradient[index] += concatenated_gradient[index];
                }
                if (layer != 0U) {
                    for (std::size_t unit = 0; unit < kUnits; ++unit) {
                        hidden_gradient[layer - 1U][time][unit] +=
                            concatenated_gradient[kEmbeddingSize + unit];
                    }
                }
                for (std::size_t unit = 0; unit < kUnits; ++unit) {
                    recurrent_hidden[layer][unit] =
                        concatenated_gradient[parameters.input_size + unit];
                    recurrent_cell[layer][unit] = previous_cell_gradient[unit];
                }
            }
            const std::size_t embedding_offset =
                static_cast<std::size_t>(sequence_symbol(result, time)) *
                kEmbeddingSize;
            for (std::size_t index = 0; index < kEmbeddingSize; ++index) {
                embedding_.gradient[embedding_offset + index] +=
                    embedding_gradient[index];
            }
        }
        apply_adam();
    }

private:
    static std::uint8_t sequence_symbol(const ForwardResult& result,
                                        const std::size_t time) {
        // Every layer sees the same embedding. The embedding itself is not
        // invertible to a symbol, so forward() stores the symbol explicitly.
        return result.cache[0][time].input_symbol;
    }

    static void initialize(ParameterVector& parameter,
                           std::uint64_t& random_state,
                           const double limit) {
        for (float& value : parameter.value) {
            value = static_cast<float>(uniform_signed(random_state) * limit);
        }
    }

    void clear_gradients() {
        embedding_.clear_gradient();
        for (LayerParameters& layer : layers_) {
            layer.weights.clear_gradient();
            layer.bias.clear_gradient();
        }
        dense_weights_.clear_gradient();
        dense_bias_.clear_gradient();
    }

    std::vector<ParameterVector*> parameters() {
        std::vector<ParameterVector*> result{&embedding_};
        for (LayerParameters& layer : layers_) {
            result.push_back(&layer.weights);
            result.push_back(&layer.bias);
        }
        result.push_back(&dense_weights_);
        result.push_back(&dense_bias_);
        return result;
    }

    void apply_adam() {
        double squared_norm = 0.0;
        for (ParameterVector* parameter : parameters()) {
            for (const double gradient : parameter->gradient) {
                squared_norm += gradient * gradient;
            }
        }
        const double norm = std::sqrt(squared_norm);
        const double scale = norm > kGradientClip && norm > 0.0
            ? kGradientClip / norm
            : 1.0;
        ++update_step_;
        const double first_correction =
            1.0 - std::pow(kAdamBeta1, static_cast<double>(update_step_));
        const double second_correction =
            1.0 - std::pow(kAdamBeta2, static_cast<double>(update_step_));
        for (ParameterVector* parameter : parameters()) {
            for (std::size_t index = 0; index < parameter->value.size(); ++index) {
                const double gradient = parameter->gradient[index] * scale;
                parameter->first_moment[index] =
                    kAdamBeta1 * parameter->first_moment[index] +
                    (1.0 - kAdamBeta1) * gradient;
                parameter->second_moment[index] =
                    kAdamBeta2 * parameter->second_moment[index] +
                    (1.0 - kAdamBeta2) * gradient * gradient;
                const double first = parameter->first_moment[index] /
                    first_correction;
                const double second = parameter->second_moment[index] /
                    second_correction;
                const double updated = static_cast<double>(parameter->value[index]) -
                    kLearningRate * first / (std::sqrt(second) + kAdamEpsilon);
                if (!std::isfinite(updated)) {
                    throw std::runtime_error(
                        "jax-compress portable model update is non-finite");
                }
                parameter->value[index] = static_cast<float>(updated);
            }
        }
    }

    ParameterVector embedding_;
    std::array<LayerParameters, kLayers> layers_;
    ParameterVector dense_weights_;
    ParameterVector dense_bias_;
    std::uint64_t update_step_ = 0U;
};

class PortableLifecycle {
public:
    hz::Cdf first_cdf() const {
        hz::ProbVector uniform{};
        hz::set_uniform_probability(uniform);
        return hz::quantize_to_cdf(uniform);
    }

    void initialize(const std::uint8_t first_symbol) {
        sequence_.fill(first_symbol);
        history_.push_back(first_symbol);
        states_.assign(kSequenceLength, ModelState{});
    }

    ForwardResult predict() {
        ModelState state = std::move(states_.front());
        states_.pop_front();
        return model_.forward(sequence_, state);
    }

    void observe(const std::uint8_t symbol, const ForwardResult& result) {
        model_.update(result, symbol);
        states_.push_back(result.final_state);
        std::copy(sequence_.begin() + 1U, sequence_.end(), sequence_.begin());
        sequence_.back() = symbol;
        history_.push_back(symbol);
        if (history_.size() % kRetrainPeriod == 0U) {
            replay_recent_history();
        }
    }

private:
    void replay_recent_history() {
        if (history_.size() <= kSequenceLength) {
            return;
        }
        const std::size_t begin = std::max(
            kSequenceLength, history_.size() > kRetrainBlock
                ? history_.size() - kRetrainBlock
                : kSequenceLength);
        for (std::size_t target = begin; target < history_.size();
             target += kRetrainStride) {
            std::array<std::uint8_t, kSequenceLength> sequence{};
            std::copy(history_.begin() + static_cast<std::ptrdiff_t>(
                          target - kSequenceLength),
                      history_.begin() + static_cast<std::ptrdiff_t>(target),
                      sequence.begin());
            const ForwardResult replay = model_.forward(sequence, ModelState{});
            model_.update(replay, history_[target]);
        }
    }

    PortableOnlineModel model_;
    std::array<std::uint8_t, kSequenceLength> sequence_{};
    std::deque<ModelState> states_;
    std::vector<std::uint8_t> history_;
};

}  // namespace

std::vector<std::uint8_t> JaxCompressPortableBackend::encode(
    const ByteView input) const {
    if (input.empty()) {
        return {};
    }
    PortableLifecycle lifecycle;
    std::ostringstream encoded(std::ios::out | std::ios::binary);
    {
        hz::ArithmeticEncoderStream coder(encoded);
        coder.write_symbol(lifecycle.first_cdf(), input[0]);
        lifecycle.initialize(input[0]);
        for (std::size_t index = 1; index < input.size(); ++index) {
            const ForwardResult prediction = lifecycle.predict();
            coder.write_symbol(hz::quantize_to_cdf(prediction.probability),
                               input[index]);
            lifecycle.observe(input[index], prediction);
        }
        coder.finish();
    }
    return as_bytes(encoded.str());
}

std::vector<std::uint8_t> JaxCompressPortableBackend::decode(
    const ByteView payload, const std::size_t expected_size) const {
    if (expected_size == 0U) {
        return {};
    }
    const std::string bytes(reinterpret_cast<const char*>(payload.data()),
                            payload.size());
    std::istringstream encoded(bytes, std::ios::in | std::ios::binary);
    hz::ArithmeticDecoderStream coder(encoded);
    PortableLifecycle lifecycle;
    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    const std::uint8_t first = coder.read_symbol(lifecycle.first_cdf());
    output.push_back(first);
    lifecycle.initialize(first);
    for (std::size_t index = 1; index < expected_size; ++index) {
        const ForwardResult prediction = lifecycle.predict();
        const std::uint8_t symbol = coder.read_symbol(
            hz::quantize_to_cdf(prediction.probability));
        output.push_back(symbol);
        lifecycle.observe(symbol, prediction);
    }
    return output;
}

}  // namespace hz::r2
