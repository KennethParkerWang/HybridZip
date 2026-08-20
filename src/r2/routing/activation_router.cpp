#include "r2/routing/activation_router.h"

#include <algorithm>

namespace hz::r2 {

RepresentationActivation StructureActivationRouter::activate(
    const StructureFeatures& structure) const {
    const bool structured = structure.entropy_bits < 7.9 &&
        (structure.printable_fraction >= 0.25 ||
         structure.repeated_window_fraction >= 0.02 ||
         structure.longest_match >= 12);
    const double delta_similarity = std::max(
        std::max(structure.delta_similarity_1, structure.delta_similarity_2),
        std::max(structure.delta_similarity_4, structure.delta_similarity_8));
    const bool numeric_correlation = delta_similarity >= 0.12 ||
        structure.zero_fraction >= 0.08;

    RepresentationActivation activation{};
    activation.bwt_zstd = structured;
    activation.bwt_mtf_zstd = structured;
    activation.bwt_rlt_zstd = structured;
    activation.x86_bcj_zstd = structure.entropy_bits < 7.95 &&
        structure.x86_branch_fraction >= 0.002;
    activation.shuffle_zstd = numeric_correlation;
    activation.bitshuffle_zstd = numeric_correlation;
    activation.delta_zstd = numeric_correlation;
    activation.fastpfor = numeric_correlation;
    return activation;
}

}  // namespace hz::r2
