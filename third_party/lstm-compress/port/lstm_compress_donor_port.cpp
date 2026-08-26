#include "lstm_compress_donor_port.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <valarray>
#include <vector>

namespace hz::r2::lstm_compress_donor {
namespace {

std::valarray<std::valarray<float>> make_matrix(std::size_t rows,
                                                 std::size_t columns) {
    std::valarray<std::valarray<float>> result(rows);
    for (std::size_t row = 0; row < rows; ++row) {
        result[row] = std::valarray<float>(columns);
    }
    return result;
}

std::valarray<std::valarray<std::valarray<float>>> make_tensor(
    std::size_t epochs, std::size_t rows, std::size_t columns) {
    std::valarray<std::valarray<std::valarray<float>>> result(epochs);
    for (std::size_t epoch = 0; epoch < epochs; ++epoch) {
        result[epoch] = make_matrix(rows, columns);
    }
    return result;
}

void zero_matrix(std::valarray<std::valarray<float>>* matrix) {
    for (std::size_t row = 0; row < matrix->size(); ++row) {
        (*matrix)[row] = 0.0F;
    }
}

// Private MSVC-compatible rand stream used by the donor to initialize weights.
class DeterministicRng {
public:
    explicit DeterministicRng(const std::uint32_t seed) : state_(seed) {}
    float uniform() {
        state_ = state_ * 214013U + 2531011U;
        return static_cast<float>((state_ >> 16U) & 0x7FFFU) / 32767.0F;
    }
private:
    std::uint32_t state_;
};

class LstmLayer {
public:
    LstmLayer(unsigned int input_size, unsigned int auxiliary_input_size,
              unsigned int output_size, unsigned int num_cells,
              unsigned int horizon, float learning_rate, float gradient_clip,
              DeterministicRng* rng)
        : state_(num_cells), output_gate_error_(num_cells),
          state_error_(num_cells), input_node_error_(num_cells),
          input_gate_error_(num_cells), forget_gate_error_(num_cells),
          stored_error_(num_cells),
          tanh_state_(std::valarray<float>(num_cells), horizon),
          output_gate_state_(std::valarray<float>(num_cells), horizon),
          input_node_state_(std::valarray<float>(num_cells), horizon),
          input_gate_state_(std::valarray<float>(num_cells), horizon),
          forget_gate_state_(std::valarray<float>(num_cells), horizon),
          last_state_(std::valarray<float>(num_cells), horizon),
          forget_gate_(std::valarray<float>(input_size), num_cells),
          input_node_(std::valarray<float>(input_size), num_cells),
          input_gate_(std::valarray<float>(input_size), num_cells),
          output_gate_(std::valarray<float>(input_size), num_cells),
          forget_gate_update_(std::valarray<float>(input_size), num_cells),
          input_node_update_(std::valarray<float>(input_size), num_cells),
          input_gate_update_(std::valarray<float>(input_size), num_cells),
          output_gate_update_(std::valarray<float>(input_size), num_cells),
          learning_rate_(learning_rate), gradient_clip_(gradient_clip),
          num_cells_(num_cells), epoch_(0), horizon_(horizon),
          input_size_(auxiliary_input_size), output_size_(output_size),
          rng_(rng) {
        constexpr float low = -0.2F;
        constexpr float range = 0.4F;
        for (unsigned int i = 0; i < num_cells_; ++i) {
            for (std::size_t j = 0; j < forget_gate_[i].size(); ++j) {
                forget_gate_[i][j] = low + rng_->uniform() * range;
                input_node_[i][j] = low + rng_->uniform() * range;
                input_gate_[i][j] = low + rng_->uniform() * range;
                output_gate_[i][j] = low + rng_->uniform() * range;
            }
            forget_gate_[i][forget_gate_[i].size() - 1U] = 1.0F;
        }
    }

    void forward(const std::valarray<float>& input, int input_symbol,
                 std::valarray<float>* hidden, int hidden_start) {
        last_state_[epoch_] = state_;
        for (unsigned int i = 0; i < num_cells_; ++i) {
            forget_gate_state_[epoch_][i] = logistic(std::inner_product(
                std::begin(input), std::end(input),
                std::begin(forget_gate_[i]) + output_size_,
                forget_gate_[i][input_symbol]));
            input_node_state_[epoch_][i] = std::tanh(std::inner_product(
                std::begin(input), std::end(input),
                std::begin(input_node_[i]) + output_size_,
                input_node_[i][input_symbol]));
            input_gate_state_[epoch_][i] = logistic(std::inner_product(
                std::begin(input), std::end(input),
                std::begin(input_gate_[i]) + output_size_,
                input_gate_[i][input_symbol]));
            output_gate_state_[epoch_][i] = logistic(std::inner_product(
                std::begin(input), std::end(input),
                std::begin(output_gate_[i]) + output_size_,
                output_gate_[i][input_symbol]));
        }
        state_ *= forget_gate_state_[epoch_];
        state_ += input_node_state_[epoch_] * input_gate_state_[epoch_];
        tanh_state_[epoch_] = state_.apply([](float value) {
            return std::tanh(value);
        });
        (*hidden)[std::slice(static_cast<std::size_t>(hidden_start),
                             num_cells_, 1U)] =
            output_gate_state_[epoch_] * tanh_state_[epoch_];
        ++epoch_;
        if (epoch_ == horizon_) epoch_ = 0;
    }

    void backward(const std::valarray<float>& input, int epoch, int layer,
                  int input_symbol, std::valarray<float>* hidden_error) {
        if (epoch == static_cast<int>(horizon_) - 1) {
            stored_error_ = *hidden_error;
            state_error_ = 0.0F;
            zero_matrix(&forget_gate_update_);
            zero_matrix(&input_node_update_);
            zero_matrix(&input_gate_update_);
            zero_matrix(&output_gate_update_);
        } else {
            stored_error_ += *hidden_error;
        }
        output_gate_error_ = tanh_state_[epoch] * stored_error_ *
            output_gate_state_[epoch] * (1.0F - output_gate_state_[epoch]);
        state_error_ += stored_error_ * output_gate_state_[epoch] *
            (1.0F - tanh_state_[epoch] * tanh_state_[epoch]);
        input_node_error_ = state_error_ * input_gate_state_[epoch] *
            (1.0F - input_node_state_[epoch] * input_node_state_[epoch]);
        input_gate_error_ = state_error_ * input_node_state_[epoch] *
            input_gate_state_[epoch] * (1.0F - input_gate_state_[epoch]);
        forget_gate_error_ = state_error_ * last_state_[epoch] *
            forget_gate_state_[epoch] * (1.0F - forget_gate_state_[epoch]);
        *hidden_error = 0.0F;
        if (layer > 0) {
            const unsigned int offset = output_size_ + num_cells_ + input_size_;
            for (unsigned int i = 0; i < num_cells_; ++i) {
                for (unsigned int j = offset; j < offset + num_cells_; ++j) {
                    (*hidden_error)[j - offset] +=
                        input_node_[i][j] * input_node_error_[i];
                    (*hidden_error)[j - offset] +=
                        input_gate_[i][j] * input_gate_error_[i];
                    (*hidden_error)[j - offset] +=
                        forget_gate_[i][j] * forget_gate_error_[i];
                    (*hidden_error)[j - offset] +=
                        output_gate_[i][j] * output_gate_error_[i];
                }
            }
        }
        if (epoch > 0) {
            state_error_ *= forget_gate_state_[epoch];
            stored_error_ = 0.0F;
            const unsigned int offset = output_size_ + input_size_;
            for (unsigned int i = 0; i < num_cells_; ++i) {
                for (unsigned int j = offset; j < offset + num_cells_; ++j) {
                    stored_error_[j - offset] +=
                        input_node_[i][j] * input_node_error_[i];
                    stored_error_[j - offset] +=
                        input_gate_[i][j] * input_gate_error_[i];
                    stored_error_[j - offset] +=
                        forget_gate_[i][j] * forget_gate_error_[i];
                    stored_error_[j - offset] +=
                        output_gate_[i][j] * output_gate_error_[i];
                }
            }
        }
        clip(&state_error_);
        clip(&stored_error_);
        clip(hidden_error);
        const std::slice slice(output_size_, input.size(), 1U);
        for (unsigned int i = 0; i < num_cells_; ++i) {
            forget_gate_update_[i][slice] +=
                (learning_rate_ * forget_gate_error_[i]) * input;
            input_node_update_[i][slice] +=
                (learning_rate_ * input_node_error_[i]) * input;
            input_gate_update_[i][slice] +=
                (learning_rate_ * input_gate_error_[i]) * input;
            output_gate_update_[i][slice] +=
                (learning_rate_ * output_gate_error_[i]) * input;
            forget_gate_update_[i][input_symbol] +=
                learning_rate_ * forget_gate_error_[i];
            input_node_update_[i][input_symbol] +=
                learning_rate_ * input_node_error_[i];
            input_gate_update_[i][input_symbol] +=
                learning_rate_ * input_gate_error_[i];
            output_gate_update_[i][input_symbol] +=
                learning_rate_ * output_gate_error_[i];
        }
        if (epoch == 0) {
            for (unsigned int i = 0; i < num_cells_; ++i) {
                forget_gate_[i] += forget_gate_update_[i];
                input_node_[i] += input_node_update_[i];
                input_gate_[i] += input_gate_update_[i];
                output_gate_[i] += output_gate_update_[i];
            }
        }
    }

private:
    static float logistic(float value) {
        return 1.0F / (1.0F + std::exp(-value));
    }
    void clip(std::valarray<float>* values) const {
        for (std::size_t i = 0; i < values->size(); ++i) {
            (*values)[i] = std::max(-gradient_clip_,
                                    std::min(gradient_clip_, (*values)[i]));
        }
    }
    std::valarray<float> state_, output_gate_error_, state_error_,
        input_node_error_, input_gate_error_, forget_gate_error_, stored_error_;
    std::valarray<std::valarray<float>> tanh_state_, output_gate_state_,
        input_node_state_, input_gate_state_, forget_gate_state_, last_state_,
        forget_gate_, input_node_, input_gate_, output_gate_,
        forget_gate_update_, input_node_update_, input_gate_update_,
        output_gate_update_;
    float learning_rate_, gradient_clip_;
    unsigned int num_cells_, epoch_, horizon_, input_size_, output_size_;
    DeterministicRng* rng_;
};

class Lstm {
public:
    Lstm(unsigned int input_size, unsigned int output_size,
         unsigned int num_cells, unsigned int num_layers, unsigned int horizon,
         float learning_rate, float gradient_clip)
        : input_history_(horizon), hidden_(num_cells * num_layers + 1U),
          hidden_error_(num_cells),
          layer_input_(make_tensor(horizon, num_layers,
                                   input_size + 1U + num_cells * 2U)),
          output_layer_(make_tensor(horizon, output_size,
                                    num_cells * num_layers + 1U)),
          output_(std::valarray<float>(1.0F / output_size, output_size), horizon),
          learning_rate_(learning_rate), num_cells_(num_cells), epoch_(0),
          horizon_(horizon), input_size_(input_size), output_size_(output_size),
          rng_(69761U) {
        hidden_[hidden_.size() - 1U] = 1.0F;
        for (unsigned int epoch = 0; epoch < horizon_; ++epoch) {
            layer_input_[epoch][0].resize(1U + num_cells_ + input_size_);
            for (unsigned int layer = 0; layer < num_layers; ++layer) {
                layer_input_[epoch][layer][layer_input_[epoch][layer].size() - 1U] =
                    1.0F;
            }
        }
        for (unsigned int layer = 0; layer < num_layers; ++layer) {
            layers_.push_back(std::make_unique<LstmLayer>(
                static_cast<unsigned int>(layer_input_[0][layer].size() + output_size_),
                input_size_, output_size_, num_cells_, horizon_, learning_rate_,
                gradient_clip, &rng_));
        }
    }
    std::valarray<float>& perceive(unsigned int input) {
        int last_epoch = static_cast<int>(epoch_) - 1;
        if (last_epoch == -1) last_epoch = static_cast<int>(horizon_) - 1;
        const unsigned int old_input = input_history_[last_epoch];
        input_history_[last_epoch] = input;
        if (epoch_ == 0) {
            for (int epoch = static_cast<int>(horizon_) - 1; epoch >= 0; --epoch) {
                for (int layer = static_cast<int>(layers_.size()) - 1;
                     layer >= 0; --layer) {
                    const unsigned int offset = static_cast<unsigned int>(layer) * num_cells_;
                    for (unsigned int i = 0; i < output_size_; ++i) {
                        const float error = i == input_history_[epoch]
                            ? (1.0F - output_[epoch][i]) : -output_[epoch][i];
                        for (std::size_t j = 0; j < hidden_error_.size(); ++j) {
                            hidden_error_[j] += output_layer_[epoch][i][j + offset] * error;
                        }
                    }
                    int previous_epoch = epoch - 1;
                    if (previous_epoch == -1) previous_epoch = static_cast<int>(horizon_) - 1;
                    unsigned int input_symbol = input_history_[previous_epoch];
                    if (epoch == 0) input_symbol = old_input;
                    layers_[layer]->backward(layer_input_[epoch][layer], epoch,
                                             layer, input_symbol, &hidden_error_);
                }
            }
        }
        output_layer_[epoch_] = output_layer_[last_epoch];
        for (unsigned int i = 0; i < output_size_; ++i) {
            const float error = i == input ? (1.0F - output_[last_epoch][i])
                                           : -output_[last_epoch][i];
            output_layer_[epoch_][i] += learning_rate_ * error * hidden_;
        }
        return predict(input);
    }
    std::valarray<float>& predict(unsigned int input) {
        for (unsigned int layer = 0; layer < layers_.size(); ++layer) {
            const auto start = std::begin(hidden_) + layer * num_cells_;
            std::copy(start, start + num_cells_,
                      std::begin(layer_input_[epoch_][layer]) + input_size_);
            layers_[layer]->forward(layer_input_[epoch_][layer], input, &hidden_,
                                    static_cast<int>(layer * num_cells_));
            if (layer < layers_.size() - 1U) {
                auto start2 = std::begin(layer_input_[epoch_][layer + 1]) +
                    num_cells_ + input_size_;
                std::copy(start, start + num_cells_, start2);
            }
        }
        float max_out = 0.0F;
        for (unsigned int i = 0; i < output_size_; ++i) {
            float sum = 0.0F;
            for (std::size_t j = 0; j < hidden_.size(); ++j) {
                sum += hidden_[j] * output_layer_[epoch_][i][j];
            }
            output_[epoch_][i] = sum;
            max_out = std::max(sum, max_out);
        }
        for (unsigned int i = 0; i < output_size_; ++i) {
            output_[epoch_][i] = std::exp(output_[epoch_][i] - max_out);
        }
        output_[epoch_] /= output_[epoch_].sum();
        const unsigned int current = epoch_;
        ++epoch_;
        if (epoch_ == horizon_) epoch_ = 0;
        return output_[current];
    }
private:
    std::vector<std::unique_ptr<LstmLayer>> layers_;
    std::vector<unsigned int> input_history_;
    std::valarray<float> hidden_, hidden_error_;
    std::valarray<std::valarray<std::valarray<float>>> layer_input_, output_layer_;
    std::valarray<std::valarray<float>> output_;
    float learning_rate_;
    unsigned int num_cells_, epoch_, horizon_, input_size_, output_size_;
    DeterministicRng rng_;
};

class ByteModel {
public:
    explicit ByteModel(const std::vector<bool>& vocab)
        : top_(0), mid_(0), bot_(0), byte_map_(0, 256), probs_(),
          bit_context_(1), vocab_(vocab) {
        unsigned int vocab_size = 0;
        for (bool present : vocab_) if (present) ++vocab_size;
        if (vocab_size == 0U) throw std::runtime_error("lstm-compress empty vocabulary");
        // The donor keeps a 256-slot byte interval.  Inactive vocabulary
        // slots become zero only after the first completed byte; collapsing
        // them to a dense interval changes the donor bit probabilities.
        probs_ = std::valarray<float>(1.0F / 256.0F, 256U);
        top_ = 255;
        int offset = 0;
        for (int i = 0; i < 256; ++i) {
            byte_map_[i] = offset;
            if (vocab_[i]) ++offset;
        }
        lstm_ = std::make_unique<Lstm>(vocab_size, vocab_size, 90, 3, 10,
                                        0.05F, 2.0F);
    }
    float predict() {
        mid_ = bot_ + ((top_ - bot_) / 2);
        float num = 0.0F;
        for (int i = mid_ + 1; i <= top_; ++i) num += probs_[i];
        float denom = num;
        for (int i = bot_; i <= mid_; ++i) denom += probs_[i];
        return denom == 0.0F ? 0.5F : num / denom;
    }
    void perceive(int bit) {
        if (bit) bot_ = mid_ + 1;
        else top_ = mid_;
        bit_context_ += bit_context_ + static_cast<unsigned int>(bit);
        if (bit_context_ >= 256U) {
            bit_context_ -= 256U;
            const auto& output = lstm_->perceive(
                static_cast<unsigned int>(byte_map_[bit_context_]));
            std::size_t offset = 0;
            for (std::size_t i = 0; i < 256; ++i) {
                if (vocab_[i]) {
                    probs_[i] = output[offset];
                    ++offset;
                } else {
                    probs_[i] = 0.0F;
                }
            }
            bit_context_ = 1U;
            top_ = static_cast<int>(probs_.size()) - 1;
            bot_ = 0;
        }
    }
private:
    int top_, mid_, bot_;
    std::valarray<int> byte_map_;
    std::valarray<float> probs_;
    unsigned int bit_context_;
    const std::vector<bool>& vocab_;
    std::unique_ptr<Lstm> lstm_;
};

unsigned int discretize(float probability) {
    probability = std::max(0.0F, std::min(1.0F, probability));
    return 1U + static_cast<unsigned int>(65534.0F * probability);
}

void encode_bit(std::vector<std::uint8_t>* output, ByteModel* predictor, int bit,
                std::uint32_t* x1, std::uint32_t* x2) {
    const unsigned int p = discretize(predictor->predict());
    const std::uint32_t range = *x2 - *x1;
    const std::uint32_t xmid = *x1 + (range >> 16U) * p +
        ((range & 0xFFFFU) * p >> 16U);
    if (bit) *x2 = xmid; else *x1 = xmid + 1U;
    predictor->perceive(bit);
    while (((*x1 ^ *x2) & 0xFF000000U) == 0U) {
        output->push_back(static_cast<std::uint8_t>(*x2 >> 24U));
        *x1 <<= 8U;
        *x2 = (*x2 << 8U) + 255U;
    }
}

class Reader {
public:
    explicit Reader(ByteView bytes) : bytes_(bytes), offset_(0), padding_(0) {}
    std::uint32_t read() {
        if (offset_ >= bytes_.size()) {
            // The upstream decoder returns zero after EOF while the range
            // interval is flushed.  Bound that compatibility padding rather
            // than accepting an unbounded truncated stream.
            if (padding_++ < 4U) return 0U;
            throw std::runtime_error("lstm-compress range payload is truncated");
        }
        return bytes_[offset_++];
    }
private:
    ByteView bytes_;
    std::size_t offset_;
    unsigned int padding_;
};

int decode_bit(ByteModel* predictor, Reader* reader, std::uint32_t* x1,
               std::uint32_t* x2, std::uint32_t* x) {
    const unsigned int p = discretize(predictor->predict());
    const std::uint32_t range = *x2 - *x1;
    const std::uint32_t xmid = *x1 + (range >> 16U) * p +
        ((range & 0xFFFFU) * p >> 16U);
    int bit = 0;
    if (*x <= xmid) { bit = 1; *x2 = xmid; }
    else *x1 = xmid + 1U;
    predictor->perceive(bit);
    while (((*x1 ^ *x2) & 0xFF000000U) == 0U) {
        *x1 <<= 8U;
        *x2 = (*x2 << 8U) + 255U;
        *x = (*x << 8U) + reader->read();
    }
    return bit;
}

}  // namespace

std::vector<std::uint8_t> encode(ByteView input) {
    const bool bitmap = input.size() >= 10000U;
    std::vector<bool> vocab(256, !bitmap);
    if (bitmap) {
        std::fill(vocab.begin(), vocab.end(), false);
        for (std::size_t i = 0; i < input.size(); ++i) vocab[input[i]] = true;
    }
    std::vector<std::uint8_t> output;
    output.reserve(input.size() + 104U);
    output.insert(output.end(), {'H', 'L', 'C', '1', 1U,
                                 static_cast<std::uint8_t>(bitmap ? 1U : 0U),
                                 0U, 0U});
    if (bitmap) {
        for (unsigned int group = 0; group < 32U; ++group) {
            std::uint8_t value = 0;
            for (unsigned int bit = 0; bit < 8U; ++bit) {
                if (vocab[group * 8U + bit]) value |=
                    static_cast<std::uint8_t>(1U << bit);
            }
            output.push_back(value);
        }
    }
    if (input.empty()) return output;
    ByteModel predictor(vocab);
    std::uint32_t x1 = 0U, x2 = 0xFFFFFFFFU;
    for (std::size_t i = 0; i < input.size(); ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            encode_bit(&output, &predictor, (input[i] >> bit) & 1,
                       &x1, &x2);
        }
    }
    while (((x1 ^ x2) & 0xFF000000U) == 0U) {
        output.push_back(static_cast<std::uint8_t>(x2 >> 24U));
        x1 <<= 8U;
        x2 = (x2 << 8U) + 255U;
    }
    output.push_back(static_cast<std::uint8_t>(x2 >> 24U));
    return output;
}

std::vector<std::uint8_t> decode(ByteView payload, std::size_t expected_size) {
    if (payload.size() < 8U || payload[0] != 'H' || payload[1] != 'L' ||
        payload[2] != 'C' || payload[3] != '1' || payload[4] != 1U ||
        payload[6] != 0U || payload[7] != 0U) {
        throw std::runtime_error("lstm-compress donor payload header is malformed");
    }
    const bool bitmap = (payload[5] & 1U) != 0U;
    if ((payload[5] & ~1U) != 0U || (expected_size >= 10000U) != bitmap) {
        throw std::runtime_error("lstm-compress donor vocabulary flag is invalid");
    }
    std::size_t offset = 8U;
    std::vector<bool> vocab(256, !bitmap);
    if (bitmap) {
        if (payload.size() < offset + 32U) {
            throw std::runtime_error("lstm-compress donor vocabulary is truncated");
        }
        std::fill(vocab.begin(), vocab.end(), false);
        for (unsigned int group = 0; group < 32U; ++group) {
            for (unsigned int bit = 0; bit < 8U; ++bit) {
                vocab[group * 8U + bit] =
                    (payload[offset + group] & (1U << bit)) != 0U;
            }
        }
        offset += 32U;
    }
    unsigned int active = 0;
    for (bool present : vocab) if (present) ++active;
    if (active == 0U) throw std::runtime_error("lstm-compress empty vocabulary");
    if (expected_size == 0U) return {};
    if (payload.size() - offset < 4U) {
        throw std::runtime_error("lstm-compress donor range payload is empty");
    }
    Reader reader(ByteView(payload.data() + offset, payload.size() - offset));
    std::uint32_t x1 = 0U, x2 = 0xFFFFFFFFU, x = 0U;
    for (int i = 0; i < 4; ++i) x = (x << 8U) + reader.read();
    ByteModel predictor(vocab);
    std::vector<std::uint8_t> output;
    output.reserve(expected_size);
    for (std::size_t i = 0; i < expected_size; ++i) {
        std::uint8_t value = 0;
        for (int bit = 7; bit >= 0; --bit) {
            value |= static_cast<std::uint8_t>(decode_bit(
                &predictor, &reader, &x1, &x2, &x) << bit);
        }
        if (!vocab[value]) {
            throw std::runtime_error("lstm-compress decoded symbol outside vocabulary");
        }
        output.push_back(value);
    }
    return output;
}

std::size_t maximum_payload_size(std::size_t input_size) {
    constexpr std::size_t overhead = 8U + 32U + 64U;
    if (input_size > std::numeric_limits<std::size_t>::max() - overhead) {
        throw std::runtime_error("lstm-compress payload bound overflow");
    }
    return input_size + overhead;
}

}  // namespace hz::r2::lstm_compress_donor
