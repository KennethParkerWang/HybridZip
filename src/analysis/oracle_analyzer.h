#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>

namespace hz {

struct LossMetric {
    std::string name;
    double total_bits = 0.0;
};

struct OracleAnalysis {
    std::uint64_t input_bytes = 0;
    std::array<LossMetric, 4> expert_losses{};
    std::vector<LossMetric> variant_losses;

    double coding_cdf_loss_bits = 0.0;
    double oracle_byte_loss_bits = 0.0;
    double oracle_region_256_loss_bits = 0.0;
    double oracle_block_4096_loss_bits = 0.0;
    double best_fixed_expert_loss_bits = 0.0;

    std::array<double, 4> average_expert_entropy_bits{};
    double average_disagreement_bits = 0.0;
    double average_ppmd_context_depth = 0.0;
    std::size_t maximum_ppmd_context_depth = 0;

    std::uint64_t match_candidate_total = 0;
    std::uint64_t match_active_bytes = 0;
    std::uint64_t match_best_length_total = 0;
};

OracleAnalysis analyze_oracle_file(
    const std::filesystem::path& input,
    std::ostream* per_byte_tsv = nullptr);

void write_oracle_summary_tsv(std::ostream& output,
                              const std::filesystem::path& input,
                              const OracleAnalysis& analysis);

}  // namespace hz
