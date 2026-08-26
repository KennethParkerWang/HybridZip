#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#include "r2/representation/transform.h"

namespace hz::r2 {

struct RepresentationActivation {
    bool bwt_zstd = false;
    bool bwt_mtf_zstd = false;
    bool bwt_rlt_zstd = false;
    bool x86_bcj_zstd = false;
    bool shuffle_zstd = false;
    bool bitshuffle_zstd = false;
    bool delta_zstd = false;
    bool delta_of_delta_zstd = false;
    bool fastpfor = false;
    bool record_transpose_zstd = false;
    bool jpeg_ls = false;
    bool flac_residual = false;
    bool wavpack = false;
    bool brotli_text = false;
    bool cmix_word_dictionary_zstd = false;
    bool rans = false;
    bool neural_lstm = false;
    bool shared_neural_lstm = false;
    bool lstm_compress = false;
    bool bgpt_shared_prior = false;
    bool jax_compress_portable = false;
    bool lmic_arithmetic = false;
    bool delta_binary_packed_zstd = false;
};

// A deterministic preselection gate. It uses only block bytes and never
// archive names, paths, or encoder-only information.
class StructureActivationRouter {
public:
    RepresentationActivation activate(const StructureFeatures& structure) const;
};

struct ExpertTelemetry {
    double recent_log_loss_16 = 0.0;
    double recent_log_loss_256 = 0.0;
    double recent_log_loss_4096 = 0.0;
    double entropy = 0.0;
    double disagreement = 0.0;
    std::uint32_t age = 0;
};

class IActivationRouter {
public:
    virtual ~IActivationRouter() = default;

    virtual std::vector<bool> active_experts(
        const StructureFeatures& structure,
        const std::vector<ExpertTelemetry>& telemetry) = 0;
};

// A deterministic family-level sleeping router.  The telemetry vector uses
// stable portfolio slots rather than archive-visible names:
//   0 statistical, 1 match, 2 neural, 3 structured, 4 numeric,
//   5 text, 6 media, 7 lz/coding.
// Layer A (StructureActivationRouter) supplies cheap byte-only gates; this
// layer suppresses a family only when its recent causal loss is stale and
// materially worse than the best family observed at the same horizon.
class HierarchicalActivationRouter final : public IActivationRouter {
public:
    std::vector<bool> active_experts(
        const StructureFeatures& structure,
        const std::vector<ExpertTelemetry>& telemetry) override;

    static std::size_t family_slot_count() noexcept { return 8; }

private:
    static bool family_gate(std::size_t slot,
                           const StructureFeatures& structure) noexcept;
};

}  // namespace hz::r2
