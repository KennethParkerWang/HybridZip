#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <system_error>

#include "analysis/oracle_analyzer.h"

namespace {

std::filesystem::path temporary_path_for(
    const std::filesystem::path& output) {
    std::filesystem::path temporary = output;
    temporary += ".tmp";
    return temporary;
}

void require_available_output(const std::filesystem::path& output) {
    if (std::filesystem::exists(output) ||
        std::filesystem::exists(temporary_path_for(output))) {
        throw std::runtime_error(
            "Refusing to overwrite an oracle output or temporary file");
    }
}

double variant_loss(const hz::OracleAnalysis& analysis,
                    const std::string_view name) {
    for (const hz::LossMetric& metric : analysis.variant_losses) {
        if (metric.name == name) {
            return metric.total_bits;
        }
    }
    throw std::logic_error("Required oracle variant is missing");
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc != 3 && argc != 4) {
            std::cerr << "Usage: hybridzip_oracle <input> <summary.tsv> "
                         "[per-byte.tsv]\n";
            return 2;
        }

        const std::filesystem::path input(argv[1]);
        const std::filesystem::path summary(argv[2]);
        const bool write_per_byte = argc == 4;
        const std::filesystem::path per_byte =
            write_per_byte ? std::filesystem::path(argv[3])
                           : std::filesystem::path{};
        require_available_output(summary);
        if (write_per_byte) {
            require_available_output(per_byte);
            if (std::filesystem::absolute(summary).lexically_normal() ==
                std::filesystem::absolute(per_byte).lexically_normal()) {
                throw std::runtime_error(
                    "Oracle summary and per-byte paths must differ");
            }
        }

        const std::filesystem::path summary_temporary =
            temporary_path_for(summary);
        const std::filesystem::path per_byte_temporary =
            write_per_byte ? temporary_path_for(per_byte)
                           : std::filesystem::path{};
        bool summary_published = false;
        bool per_byte_published = false;
        try {
            std::ofstream per_byte_output;
            if (write_per_byte) {
                per_byte_output.open(per_byte_temporary,
                                     std::ios::binary | std::ios::trunc);
                if (!per_byte_output) {
                    throw std::runtime_error(
                        "Failed to open per-byte oracle output");
                }
            }
            const hz::OracleAnalysis analysis = hz::analyze_oracle_file(
                input, write_per_byte ? &per_byte_output : nullptr);
            if (write_per_byte) {
                per_byte_output.flush();
                if (!per_byte_output) {
                    throw std::runtime_error(
                        "Failed to flush per-byte oracle output");
                }
                per_byte_output.close();
            }

            std::ofstream summary_output(
                summary_temporary, std::ios::binary | std::ios::trunc);
            if (!summary_output) {
                throw std::runtime_error("Failed to open oracle summary");
            }
            hz::write_oracle_summary_tsv(summary_output, input, analysis);
            summary_output.close();

            if (write_per_byte) {
                std::filesystem::rename(per_byte_temporary, per_byte);
                per_byte_published = true;
            }
            std::filesystem::rename(summary_temporary, summary);
            summary_published = true;

            std::cout << std::setprecision(12)
                      << "Oracle bytes=" << analysis.input_bytes
                      << " oracle_bpb="
                      << (analysis.input_bytes == 0U
                              ? 0.0
                              : analysis.oracle_byte_loss_bits /
                                    static_cast<double>(analysis.input_bytes))
                      << " best_fixed_bpb="
                      << (analysis.input_bytes == 0U
                              ? 0.0
                              : analysis.best_fixed_expert_loss_bits /
                                    static_cast<double>(analysis.input_bytes))
                      << " v1_bpb="
                      << (analysis.input_bytes == 0U
                              ? 0.0
                              : variant_loss(analysis,
                                             "v1-current-mixer") /
                                    static_cast<double>(analysis.input_bytes))
                      << " hedge_bpb="
                      << (analysis.input_bytes == 0U
                              ? 0.0
                              : variant_loss(analysis, "causal-hedge") /
                                    static_cast<double>(analysis.input_bytes))
                      << '\n';
            return 0;
        } catch (...) {
            std::error_code ignored;
            std::filesystem::remove(summary_temporary, ignored);
            if (write_per_byte) {
                std::filesystem::remove(per_byte_temporary, ignored);
            }
            if (summary_published) {
                std::filesystem::remove(summary, ignored);
            }
            if (per_byte_published) {
                std::filesystem::remove(per_byte, ignored);
            }
            throw;
        }
    } catch (const std::exception& error) {
        std::cerr << "hybridzip_oracle: " << error.what() << '\n';
        return 1;
    }
}
