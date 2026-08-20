#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "analysis/oracle_analyzer.h"

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

double variant_loss(const hz::OracleAnalysis& analysis,
                    const std::string& name) {
    for (const hz::LossMetric& metric : analysis.variant_losses) {
        if (metric.name == name) {
            return metric.total_bits;
        }
    }
    throw std::runtime_error("Oracle test variant is missing");
}

}  // namespace

int main() {
    std::filesystem::path input;
    try {
        input = std::filesystem::temp_directory_path() /
                "hybridzip-oracle-analyzer-test.input";
        if (std::filesystem::exists(input)) {
            throw std::runtime_error("Oracle test input already exists");
        }
        constexpr char kText[] =
            "AabcdefghXBabcdefghXCabcdefghXDabcdefghX";
        {
            std::ofstream output(input, std::ios::binary | std::ios::trunc);
            output.write(kText, sizeof(kText) - 1U);
            if (!output) {
                throw std::runtime_error("Failed to write oracle test input");
            }
        }

        std::ostringstream per_byte;
        const hz::OracleAnalysis analysis =
            hz::analyze_oracle_file(input, &per_byte);
        require(analysis.input_bytes == sizeof(kText) - 1U,
                "Oracle byte count is wrong");
        require(analysis.expert_losses.size() == 4 &&
                    analysis.variant_losses.size() == 16,
                "Oracle variant coverage is wrong");
        require(analysis.oracle_byte_loss_bits <=
                        analysis.oracle_region_256_loss_bits + 1e-10 &&
                    analysis.oracle_region_256_loss_bits <=
                        analysis.best_fixed_expert_loss_bits + 1e-10,
                "Oracle diagnostic bounds are inconsistent");
        require(std::abs(variant_loss(analysis, "v1-current-mixer") -
                         variant_loss(analysis, "causal-hedge")) < 1e-9,
                "Causal Hedge does not reproduce V1 weights");
        require(std::isfinite(analysis.average_disagreement_bits) &&
                    analysis.average_disagreement_bits >= 0.0 &&
                    analysis.maximum_ppmd_context_depth <= 12U,
                "Oracle diagnostics are invalid");

        std::istringstream per_byte_rows(per_byte.str());
        std::string line;
        std::size_t line_count = 0;
        while (std::getline(per_byte_rows, line)) {
            ++line_count;
        }
        require(line_count == analysis.input_bytes + 1U,
                "Per-byte oracle TSV row count is wrong");

        std::ostringstream summary;
        hz::write_oracle_summary_tsv(summary, input, analysis);
        require(summary.str().find("multi-timescale-hedge") !=
                    std::string::npos &&
                    summary.str().find(
                        "average-jensen-shannon-disagreement") !=
                        std::string::npos,
                "Oracle summary is missing required metrics");

        std::error_code ignored;
        std::filesystem::remove(input, ignored);
        std::cout << "oracle_analyzer_tests: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::error_code ignored;
        if (!input.empty()) {
            std::filesystem::remove(input, ignored);
        }
        std::cerr << "oracle_analyzer_tests: FAIL: " << error.what() << '\n';
        return 1;
    }
}
