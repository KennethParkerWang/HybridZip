#pragma once

#include <cstddef>
#include <cstdint>

#include "core/types.h"

namespace hz {

struct Profile {
    std::uint32_t id = 1;
    std::size_t history_capacity = 8U * 1024U * 1024U;

    std::size_t ngram_max_order = 4;
    std::size_t ngram_memory_bytes = 64U * 1024U * 1024U;
    double ngram_lambda_bias = 32.0;

    int ppmd_order = 12;
    std::size_t ppmd_memory_bytes = 64U * 1024U * 1024U;

    std::size_t match_context_bytes = 8;
    std::size_t match_hash_bits = 20;
    std::size_t match_min_length = 3;
    std::size_t match_confidence_buckets = 256;

    std::size_t lstm_output_size = kAlphabet;
    std::size_t lstm_cells = 200;
    std::size_t lstm_layers = 2;
    std::size_t lstm_horizon = 100;
    double lstm_learning_rate = 0.03;
    double lstm_gradient_clip = 10.0;

    std::size_t expert_count = 4;
    double mixer_eta = 0.5;
    std::uint32_t cdf_bits = kCdfBits;
    int coder_state_bits = kCoderStateBits;
    std::uint64_t model_seed = kDefaultModelSeed;
};

Profile make_profile_v1();

}  // namespace hz
