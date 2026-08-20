#include "analysis/oracle_analyzer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "analysis/pipeline_evidence.h"
#include "analysis/variant_mixer.h"
#include "codec/model_pipeline.h"
#include "core/probability.h"
#include "core/profile.h"
#include "core/types.h"

namespace hz {
namespace {

constexpr std::size_t kExpertCount = 4;
constexpr ExpertMask kAllExperts = 0x0FU;
constexpr std::array<const char*, kExpertCount> kExpertNames{
    "ngram", "ppmd", "match", "lstm"};

double finite_probability(const double probability) noexcept {
    if (!std::isfinite(probability) || probability < kProbFloor) {
        return kProbFloor;
    }
    return probability;
}

double symbol_loss_bits(const ProbVector& probability,
                        const std::uint8_t actual) noexcept {
    return -std::log2(finite_probability(probability[actual]));
}

double entropy_bits(const ProbVector& probability) noexcept {
    double entropy = 0.0;
    for (const double raw : probability) {
        const double value = finite_probability(raw);
        entropy -= value * std::log2(value);
    }
    return entropy;
}

double minimum_loss(const std::array<double, kExpertCount>& losses) noexcept {
    return *std::min_element(losses.begin(), losses.end());
}

struct EqualVariantState {
    EqualVariantState(std::string variant_name, const ExpertMask mask)
        : name(std::move(variant_name)), mixer(kExpertCount, mask) {}

    std::string name;
    ActiveEqualMixer mixer;
    double total_loss_bits = 0.0;
};

class OracleAccumulator final : public PipelineEvidenceSink {
public:
    explicit OracleAccumulator(std::ostream* const per_byte_tsv)
        : hedge_(kExpertCount, kAllExperts,
                 HedgeScaleConfig{0.5, 1.0}),
          multi_(kExpertCount, kAllExperts,
                 std::vector<HedgeScaleConfig>{
                     HedgeScaleConfig{0.5, discount_from_half_life(16.0)},
                     HedgeScaleConfig{0.5, discount_from_half_life(256.0)},
                     HedgeScaleConfig{0.5, discount_from_half_life(4096.0)},
                     HedgeScaleConfig{0.5, 1.0}}),
          per_byte_tsv_(per_byte_tsv) {
        equal_variants_.emplace_back("ngram+ppmd", 0x03U);
        equal_variants_.emplace_back("ppmd+match", 0x06U);
        equal_variants_.emplace_back("ppmd+lstm", 0x0AU);
        equal_variants_.emplace_back("match+lstm", 0x0CU);
        equal_variants_.emplace_back("leave-ngram-out", 0x0EU);
        equal_variants_.emplace_back("leave-ppmd-out", 0x0DU);
        equal_variants_.emplace_back("leave-match-out", 0x0BU);
        equal_variants_.emplace_back("leave-lstm-out", 0x07U);
        equal_variants_.emplace_back("equal-mixture", kAllExperts);

        if (per_byte_tsv_ != nullptr) {
            *per_byte_tsv_
                << "position\tactual\tngram_nll_bits\tppmd_nll_bits"
                   "\tmatch_nll_bits\tlstm_nll_bits\tv1_nll_bits"
                   "\tcoding_cdf_nll_bits\tcausal_hedge_nll_bits"
                   "\tmulti_timescale_hedge_nll_bits"
                   "\tngram_entropy_bits\tppmd_entropy_bits"
                   "\tmatch_entropy_bits\tlstm_entropy_bits"
                   "\tdisagreement_bits\tv1_weight_ngram"
                   "\tv1_weight_ppmd\tv1_weight_match\tv1_weight_lstm"
                   "\thedge_weight_ngram\thedge_weight_ppmd"
                   "\thedge_weight_match\thedge_weight_lstm"
                   "\tmulti_weight_ngram\tmulti_weight_ppmd"
                   "\tmulti_weight_match\tmulti_weight_lstm"
                   "\tmatch_candidate_count\tmatch_best_length"
                   "\tmatch_prediction_active\tmatch_candidate_symbol"
                   "\tmatch_candidate_probability\tppmd_context_depth"
                   "\tblock_local_v1_loss_bits\n";
            *per_byte_tsv_ << std::setprecision(17);
        }
    }

    void on_byte(const PipelineByteEvidence& evidence) override {
        if (finished_) {
            throw std::logic_error("Oracle evidence arrived after finish");
        }
        if (evidence.position != input_bytes_ ||
            evidence.expert_probabilities.size() != kExpertCount ||
            evidence.mixer_weights.size() != kExpertCount) {
            throw std::runtime_error("Oracle evidence sequence is invalid");
        }

        std::array<double, kExpertCount> losses{};
        std::array<double, kExpertCount> entropies{};
        ProbVector centroid{};
        centroid.fill(0.0);
        for (std::size_t expert = 0; expert < kExpertCount; ++expert) {
            const ProbVector& probability =
                evidence.expert_probabilities[expert];
            losses[expert] = symbol_loss_bits(probability, evidence.actual);
            entropies[expert] = entropy_bits(probability);
            expert_loss_bits_[expert] += losses[expert];
            expert_entropy_bits_[expert] += entropies[expert];
            region_256_loss_bits_[expert] += losses[expert];
            block_4096_loss_bits_[expert] += losses[expert];
            for (std::size_t symbol = 0; symbol < kAlphabet; ++symbol) {
                centroid[symbol] +=
                    probability[symbol] / static_cast<double>(kExpertCount);
            }
        }

        const double average_entropy =
            (entropies[0] + entropies[1] + entropies[2] + entropies[3]) /
            static_cast<double>(kExpertCount);
        disagreement_bits_ +=
            std::max(0.0, entropy_bits(centroid) - average_entropy);

        oracle_byte_loss_bits_ += minimum_loss(losses);
        const double v1_loss =
            symbol_loss_bits(evidence.mixed_probability, evidence.actual);
        v1_loss_bits_ += v1_loss;
        block_local_v1_loss_bits_ += v1_loss;

        const std::uint32_t coding_frequency =
            evidence.coding_cdf.v[evidence.actual + 1U] -
            evidence.coding_cdf.v[evidence.actual];
        if (coding_frequency == 0U) {
            throw std::runtime_error("Oracle evidence has a zero CDF bin");
        }
        const double coding_loss = -std::log2(
            static_cast<double>(coding_frequency) /
            static_cast<double>(kCdfTotal));
        coding_cdf_loss_bits_ += coding_loss;

        for (EqualVariantState& variant : equal_variants_) {
            ProbVector probability{};
            variant.mixer.mix(evidence.expert_probabilities, probability);
            variant.total_loss_bits +=
                symbol_loss_bits(probability, evidence.actual);
        }

        ProbVector hedge_probability{};
        hedge_.mix(evidence.expert_probabilities, hedge_probability);
        const double hedge_loss =
            symbol_loss_bits(hedge_probability, evidence.actual);
        hedge_loss_bits_ += hedge_loss;

        ProbVector multi_probability{};
        multi_.mix(evidence.expert_probabilities, multi_probability);
        const double multi_loss =
            symbol_loss_bits(multi_probability, evidence.actual);
        multi_loss_bits_ += multi_loss;

        match_candidate_total_ += evidence.match.candidate_count;
        match_best_length_total_ += evidence.match.best_match_length;
        if (evidence.match.prediction_active) {
            ++match_active_bytes_;
        }
        ppmd_context_depth_total_ += evidence.ppmd_context_depth;
        maximum_ppmd_context_depth_ = std::max(
            maximum_ppmd_context_depth_, evidence.ppmd_context_depth);

        if (per_byte_tsv_ != nullptr) {
            write_per_byte(evidence, losses, entropies, v1_loss, coding_loss,
                           hedge_loss, multi_loss);
        }

        hedge_.update(evidence.actual, evidence.expert_probabilities);
        multi_.update(evidence.actual, evidence.expert_probabilities);
        ++input_bytes_;
        ++region_256_bytes_;
        ++block_4096_bytes_;
        if (region_256_bytes_ == 256U) {
            close_region(region_256_loss_bits_, region_256_bytes_,
                         oracle_region_256_loss_bits_);
        }
        if (block_4096_bytes_ == 4096U) {
            close_region(block_4096_loss_bits_, block_4096_bytes_,
                         oracle_block_4096_loss_bits_);
            block_local_v1_loss_bits_ = 0.0;
        }
    }

    OracleAnalysis finish() {
        if (finished_) {
            throw std::logic_error("Oracle analysis was finished twice");
        }
        if (region_256_bytes_ != 0U) {
            close_region(region_256_loss_bits_, region_256_bytes_,
                         oracle_region_256_loss_bits_);
        }
        if (block_4096_bytes_ != 0U) {
            close_region(block_4096_loss_bits_, block_4096_bytes_,
                         oracle_block_4096_loss_bits_);
        }
        if (per_byte_tsv_ != nullptr && !*per_byte_tsv_) {
            throw std::runtime_error("Failed to write per-byte oracle TSV");
        }
        finished_ = true;

        OracleAnalysis result{};
        result.input_bytes = input_bytes_;
        for (std::size_t expert = 0; expert < kExpertCount; ++expert) {
            result.expert_losses[expert] =
                LossMetric{kExpertNames[expert], expert_loss_bits_[expert]};
            result.variant_losses.push_back(LossMetric{
                std::string(kExpertNames[expert]) + "-only",
                expert_loss_bits_[expert]});
        }
        for (const EqualVariantState& variant : equal_variants_) {
            result.variant_losses.push_back(
                LossMetric{variant.name, variant.total_loss_bits});
        }
        result.variant_losses.push_back(
            LossMetric{"v1-current-mixer", v1_loss_bits_});
        result.variant_losses.push_back(
            LossMetric{"causal-hedge", hedge_loss_bits_});
        result.variant_losses.push_back(
            LossMetric{"multi-timescale-hedge", multi_loss_bits_});

        result.coding_cdf_loss_bits = coding_cdf_loss_bits_;
        result.oracle_byte_loss_bits = oracle_byte_loss_bits_;
        result.oracle_region_256_loss_bits =
            oracle_region_256_loss_bits_;
        result.oracle_block_4096_loss_bits =
            oracle_block_4096_loss_bits_;
        result.best_fixed_expert_loss_bits = minimum_loss(expert_loss_bits_);
        if (input_bytes_ != 0U) {
            const double scale = 1.0 / static_cast<double>(input_bytes_);
            for (std::size_t expert = 0; expert < kExpertCount; ++expert) {
                result.average_expert_entropy_bits[expert] =
                    expert_entropy_bits_[expert] * scale;
            }
            result.average_disagreement_bits = disagreement_bits_ * scale;
            result.average_ppmd_context_depth =
                static_cast<double>(ppmd_context_depth_total_) * scale;
        }
        result.maximum_ppmd_context_depth = maximum_ppmd_context_depth_;
        result.match_candidate_total = match_candidate_total_;
        result.match_active_bytes = match_active_bytes_;
        result.match_best_length_total = match_best_length_total_;
        return result;
    }

private:
    static void close_region(
        std::array<double, kExpertCount>& region_losses,
        std::size_t& region_bytes,
        double& oracle_total) {
        oracle_total += minimum_loss(region_losses);
        region_losses.fill(0.0);
        region_bytes = 0;
    }

    void write_per_byte(
        const PipelineByteEvidence& evidence,
        const std::array<double, kExpertCount>& losses,
        const std::array<double, kExpertCount>& entropies,
        const double v1_loss,
        const double coding_loss,
        const double hedge_loss,
        const double multi_loss) {
        *per_byte_tsv_ << evidence.position << '\t'
                       << static_cast<unsigned>(evidence.actual);
        for (const double loss : losses) {
            *per_byte_tsv_ << '\t' << loss;
        }
        *per_byte_tsv_ << '\t' << v1_loss << '\t' << coding_loss << '\t'
                       << hedge_loss << '\t' << multi_loss;
        for (const double entropy : entropies) {
            *per_byte_tsv_ << '\t' << entropy;
        }
        const double average_entropy =
            (entropies[0] + entropies[1] + entropies[2] + entropies[3]) /
            static_cast<double>(kExpertCount);
        const ProbVector centroid = centroid_for(
            evidence.expert_probabilities);
        *per_byte_tsv_ << '\t'
                       << std::max(
                              0.0, entropy_bits(centroid) - average_entropy);
        for (const double weight : evidence.mixer_weights) {
            *per_byte_tsv_ << '\t' << weight;
        }
        for (const double weight : hedge_.weights()) {
            *per_byte_tsv_ << '\t' << weight;
        }
        for (const double weight : multi_.weights()) {
            *per_byte_tsv_ << '\t' << weight;
        }
        *per_byte_tsv_
            << '\t' << evidence.match.candidate_count
            << '\t' << evidence.match.best_match_length
            << '\t' << (evidence.match.prediction_active ? 1 : 0)
            << '\t' << static_cast<unsigned>(evidence.match.candidate_symbol)
            << '\t' << evidence.match.candidate_probability
            << '\t' << evidence.ppmd_context_depth
            << '\t' << block_local_v1_loss_bits_ << '\n';
    }

    static ProbVector centroid_for(
        const std::vector<ProbVector>& probabilities) {
        ProbVector centroid{};
        centroid.fill(0.0);
        for (const ProbVector& probability : probabilities) {
            for (std::size_t symbol = 0; symbol < kAlphabet; ++symbol) {
                centroid[symbol] += probability[symbol] /
                                    static_cast<double>(kExpertCount);
            }
        }
        return centroid;
    }

    std::vector<EqualVariantState> equal_variants_;
    DiscountedHedgeMixer hedge_;
    MultiTimescaleHedgeMixer multi_;
    std::ostream* per_byte_tsv_ = nullptr;

    std::uint64_t input_bytes_ = 0;
    std::array<double, kExpertCount> expert_loss_bits_{};
    std::array<double, kExpertCount> expert_entropy_bits_{};
    double v1_loss_bits_ = 0.0;
    double coding_cdf_loss_bits_ = 0.0;
    double hedge_loss_bits_ = 0.0;
    double multi_loss_bits_ = 0.0;
    double oracle_byte_loss_bits_ = 0.0;
    double oracle_region_256_loss_bits_ = 0.0;
    double oracle_block_4096_loss_bits_ = 0.0;
    double disagreement_bits_ = 0.0;

    std::array<double, kExpertCount> region_256_loss_bits_{};
    std::array<double, kExpertCount> block_4096_loss_bits_{};
    std::size_t region_256_bytes_ = 0;
    std::size_t block_4096_bytes_ = 0;
    double block_local_v1_loss_bits_ = 0.0;

    std::uint64_t match_candidate_total_ = 0;
    std::uint64_t match_active_bytes_ = 0;
    std::uint64_t match_best_length_total_ = 0;
    std::uint64_t ppmd_context_depth_total_ = 0;
    std::size_t maximum_ppmd_context_depth_ = 0;
    bool finished_ = false;
};

std::string tsv_text(std::string value) {
    for (char& character : value) {
        if (character == '\t' || character == '\r' || character == '\n') {
            character = ' ';
        }
    }
    return value;
}

double bits_per_byte(const double bits, const std::uint64_t bytes) noexcept {
    return bytes == 0U ? 0.0 : bits / static_cast<double>(bytes);
}

}  // namespace

OracleAnalysis analyze_oracle_file(const std::filesystem::path& input,
                                   std::ostream* const per_byte_tsv) {
    if (!std::filesystem::is_regular_file(input)) {
        throw std::runtime_error("Oracle input is not a regular file");
    }
    const std::uintmax_t expected_size = std::filesystem::file_size(input);
    if (expected_size > std::numeric_limits<std::uint64_t>::max()) {
        throw std::runtime_error("Oracle input is too large");
    }

    std::ifstream source(input, std::ios::binary);
    if (!source) {
        throw std::runtime_error("Failed to open oracle input");
    }
    const Profile profile = make_profile_v1();
    ModelPipeline pipeline(profile);
    pipeline.reset(profile.model_seed);
    OracleAccumulator accumulator(per_byte_tsv);
    pipeline.set_evidence_sink(&accumulator);

    std::uint64_t processed = 0;
    char value = 0;
    while (source.get(value)) {
        if (processed >= expected_size) {
            throw std::runtime_error("Oracle input grew during analysis");
        }
        const std::uint8_t actual = static_cast<std::uint8_t>(
            static_cast<unsigned char>(value));
        pipeline.predict_cdf();
        pipeline.observe(actual);
        ++processed;
    }
    if (source.bad()) {
        throw std::runtime_error("Failed while reading oracle input");
    }
    if (processed != expected_size) {
        throw std::runtime_error("Oracle input shrank during analysis");
    }
    return accumulator.finish();
}

void write_oracle_summary_tsv(std::ostream& output,
                              const std::filesystem::path& input,
                              const OracleAnalysis& analysis) {
    output << "input_path\tinput_bytes\tcategory\tname\ttotal_bits"
              "\tbits_per_byte\tbest_fixed_regret_bits\toracle_gap_bits"
              "\tvalue\tunit\n"
           << std::setprecision(17);
    const std::string input_text = tsv_text(input.string());
    auto write_loss = [&](const char* category, const LossMetric& metric) {
        output << input_text << '\t' << analysis.input_bytes << '\t'
               << category << '\t' << metric.name << '\t'
               << metric.total_bits << '\t'
               << bits_per_byte(metric.total_bits, analysis.input_bytes)
               << '\t'
               << metric.total_bits - analysis.best_fixed_expert_loss_bits
               << '\t'
               << metric.total_bits - analysis.oracle_byte_loss_bits
               << "\tNA\tbits\n";
    };
    for (const LossMetric& expert : analysis.expert_losses) {
        write_loss("expert", expert);
    }
    for (const LossMetric& variant : analysis.variant_losses) {
        write_loss("variant", variant);
    }
    write_loss("coding", LossMetric{"v1-quantized-cdf",
                                     analysis.coding_cdf_loss_bits});
    write_loss("oracle", LossMetric{"oracle-byte",
                                     analysis.oracle_byte_loss_bits});
    write_loss("oracle", LossMetric{"oracle-region-256",
                                     analysis.oracle_region_256_loss_bits});
    write_loss("oracle", LossMetric{"oracle-block-4096",
                                     analysis.oracle_block_4096_loss_bits});
    write_loss("oracle", LossMetric{"best-fixed-expert",
                                     analysis.best_fixed_expert_loss_bits});

    auto write_value = [&](const std::string& name,
                           const double value,
                           const char* unit) {
        output << input_text << '\t' << analysis.input_bytes
               << "\tdiagnostic\t" << name
               << "\tNA\tNA\tNA\tNA\t" << value << '\t' << unit
               << '\n';
    };
    for (std::size_t expert = 0; expert < kExpertCount; ++expert) {
        write_value(std::string(kExpertNames[expert]) +
                        "-average-entropy",
                    analysis.average_expert_entropy_bits[expert],
                    "bits-per-symbol");
    }
    write_value("average-jensen-shannon-disagreement",
                analysis.average_disagreement_bits, "bits-per-symbol");
    write_value("average-ppmd-context-depth",
                analysis.average_ppmd_context_depth, "bytes");
    write_value("maximum-ppmd-context-depth",
                static_cast<double>(analysis.maximum_ppmd_context_depth),
                "bytes");
    const double inverse_size = analysis.input_bytes == 0U
                                    ? 0.0
                                    : 1.0 / static_cast<double>(
                                          analysis.input_bytes);
    write_value("match-candidates-per-byte",
                static_cast<double>(analysis.match_candidate_total) *
                    inverse_size,
                "candidates-per-byte");
    write_value("match-active-fraction",
                static_cast<double>(analysis.match_active_bytes) *
                    inverse_size,
                "fraction");
    write_value("match-average-best-length",
                static_cast<double>(analysis.match_best_length_total) *
                    inverse_size,
                "bytes");
    if (!output) {
        throw std::runtime_error("Failed to write oracle summary TSV");
    }
}

}  // namespace hz
