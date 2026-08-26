#include "r2/entropy/paq8px_generic_sse_backend.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "r2/entropy/binary_arithmetic_codec.h"

#include "Mixer_Scalar.hpp"
#include "SSE.hpp"
#include "Shared.hpp"
#include "model/CharGroupModel.hpp"
#include "model/ChartModel.hpp"
#include "model/DmcForest.hpp"
#include "model/ExeModel.hpp"
#include "model/IndirectModel.hpp"
#include "model/LinearPredictionModel.hpp"
#include "model/MatchModel.hpp"
#include "model/NestModel.hpp"
#include "model/NormalModel.hpp"
#include "model/RecordModel.hpp"
#include "model/SimilarityModel.hpp"
#include "model/SimilarityModelPair.hpp"
#include "model/SparseBitModel.hpp"
#include "model/SparseMatchModel.hpp"
#include "model/SparseModel.hpp"
#include "model/WordModel.hpp"
#include "model/XMLModel.hpp"
#include "text/TextModel.hpp"

namespace hz::r2 {
namespace {

constexpr std::uint32_t kProbabilityScale = 1U << 24U;
constexpr std::uint32_t kDonorPrecision = 31U;
constexpr int kDonorLevel = 1;
constexpr std::uint32_t kDonorBufferSize = 1U << 20U;

constexpr int kMixerInputs =
    1 + MatchModel::MIXERINPUTS + NormalModel::MIXERINPUTS +
    SparseMatchModel::MIXERINPUTS + SparseModel::MIXERINPUTS +
    SparseBitModel::MIXERINPUTS + ChartModel::MIXERINPUTS +
    RecordModel::MIXERINPUTS + CharGroupModel::MIXERINPUTS +
    TextModel::MIXERINPUTS + WordModel::MIXERINPUTS_BIN +
    IndirectModel::MIXERINPUTS + DmcForest::MIXERINPUTS +
    NestModel::MIXERINPUTS + XMLModel::MIXERINPUTS +
    LinearPredictionModel::MIXERINPUTS + 2 * SimilarityModel::MIXERINPUTS +
    ExeModel::MIXERINPUTS;

constexpr int kMixerContexts =
    MatchModel::MIXERCONTEXTS + NormalModel::MIXERCONTEXTS_PRE +
    NormalModel::MIXERCONTEXTS_POST + SparseMatchModel::MIXERCONTEXTS +
    SparseModel::MIXERCONTEXTS + SparseBitModel::MIXERCONTEXTS +
    ChartModel::MIXERCONTEXTS + RecordModel::MIXERCONTEXTS +
    CharGroupModel::MIXERCONTEXTS + TextModel::MIXERCONTEXTS +
    WordModel::MIXERCONTEXTS + IndirectModel::MIXERCONTEXTS +
    DmcForest::MIXERCONTEXTS + NestModel::MIXERCONTEXTS +
    XMLModel::MIXERCONTEXTS + LinearPredictionModel::MIXERCONTEXTS +
    2 * SimilarityModel::MIXERCONTEXTS + ExeModel::MIXERCONTEXTS;

constexpr int kMixerContextSets =
    MatchModel::MIXERCONTEXTSETS + NormalModel::MIXERCONTEXTSETS_PRE +
    NormalModel::MIXERCONTEXTSETS_POST + SparseMatchModel::MIXERCONTEXTSETS +
    SparseModel::MIXERCONTEXTSETS + SparseBitModel::MIXERCONTEXTSETS +
    ChartModel::MIXERCONTEXTSETS + RecordModel::MIXERCONTEXTSETS +
    CharGroupModel::MIXERCONTEXTSETS + TextModel::MIXERCONTEXTSETS +
    WordModel::MIXERCONTEXTSETS + IndirectModel::MIXERCONTEXTSETS +
    DmcForest::MIXERCONTEXTSETS + NestModel::MIXERCONTEXTSETS +
    XMLModel::MIXERCONTEXTSETS + LinearPredictionModel::MIXERCONTEXTSETS +
    2 * SimilarityModel::MIXERCONTEXTSETS + ExeModel::MIXERCONTEXTSETS;

class GenericSseSession {
public:
  explicit GenericSseSession(const std::size_t block_size) {
    if (block_size > std::numeric_limits<std::uint32_t>::max()) {
      throw std::length_error("PAQ8px Generic+SSE block exceeds 32-bit size");
    }

    shared_.init(kDonorLevel, kDonorBufferSize);
    shared_.chosenSimd = SIMDType::SIMD_NONE;
    shared_.State.blockType = BlockType::DEFAULT;
    shared_.State.blockInfo = -1;
    shared_.State.blockPos = std::numeric_limits<std::uint32_t>::max();
    shared_.State.Match.expectedByte = 256U;

    // ContextModelGeneric creates the mixer first, then lazily creates
    // models in this exact order on its first p() call.
    mixer_ = std::make_unique<Mixer_Scalar>(
        &shared_, kMixerInputs, kMixerContexts, kMixerContextSets, 0);
    mixer_->setScaleFactor(980, 90);
    normal_ = std::make_unique<NormalModel>(&shared_, shared_.mem * 32U);
    match_ =
        std::make_unique<MatchModel>(&shared_, shared_.mem / 4U, shared_.mem);
    sparse_match_ = std::make_unique<SparseMatchModel>(&shared_, shared_.mem);
    sparse_bit_ = std::make_unique<SparseBitModel>(&shared_, shared_.mem / 4U);
    sparse_ = std::make_unique<SparseModel>(&shared_, shared_.mem * 4U);
    chart_ = std::make_unique<ChartModel>(&shared_, shared_.mem * 4U);
    record_ = std::make_unique<RecordModel>(&shared_, shared_.mem * 2U);
    char_group_ = std::make_unique<CharGroupModel>(&shared_, shared_.mem / 2U);
    text_ = std::make_unique<TextModel>(&shared_, shared_.mem * 16U);
    word_ = std::make_unique<WordModel>(&shared_, shared_.mem * 16U);
    indirect_ = std::make_unique<IndirectModel>(&shared_, shared_.mem * 2U);
    dmc_ = std::make_unique<DmcForest>(&shared_, shared_.mem);
    nest_ = std::make_unique<NestModel>(&shared_, shared_.mem);
    xml_ = std::make_unique<XMLModel>(&shared_, shared_.mem / 4U);
    linear_ = std::make_unique<LinearPredictionModel>(&shared_);
    similarity_ = std::make_unique<SimilarityModelPair>(&shared_, shared_.mem);
    exe_ = std::make_unique<ExeModel>(&shared_, shared_.mem * 4U);
    sse_ = std::make_unique<SSE>(&shared_);
  }

  std::uint32_t predict() {
    if (shared_.State.bitPosition == 0U) {
      ++shared_.State.blockPos;
    }

    mixer_->add(256);
    normal_->mix(*mixer_);
    normal_->mixPost(*mixer_);
    match_->mix(*mixer_);
    sparse_match_->mix(*mixer_);
    sparse_bit_->mix(*mixer_);
    sparse_->mix(*mixer_);
    chart_->mix(*mixer_);
    record_->mix(*mixer_);
    char_group_->mix(*mixer_);
    text_->mix(*mixer_);
    word_->mix(*mixer_);
    indirect_->mix(*mixer_);
    dmc_->mix(*mixer_);
    nest_->mix(*mixer_);
    xml_->mix(*mixer_);
    linear_->mix(*mixer_);
    similarity_->mix(*mixer_);
    exe_->mix(*mixer_);

    const int p12 = std::clamp(mixer_->p(), 1, 4095);
    const std::uint32_t p31 = sse_->p(static_cast<std::uint32_t>(p12));
    return std::clamp(p31 >> (kDonorPrecision - 24U), 1U,
                      kProbabilityScale - 1U);
  }

  void observe(const std::uint8_t bit, const std::uint32_t p24) {
    if (bit > 1U || p24 == 0U || p24 >= kProbabilityScale) {
      throw std::invalid_argument(
          "PAQ8px Generic+SSE received invalid bit state");
    }
    const std::uint32_t p31 = p24 << (kDonorPrecision - 24U);
    const bool is_missed = static_cast<std::uint8_t>(p31 >> 30U) != bit;
    shared_.update(bit, p31, is_missed);
  }

private:
  Shared shared_;
  std::unique_ptr<Mixer_Scalar> mixer_;
  std::unique_ptr<NormalModel> normal_;
  std::unique_ptr<MatchModel> match_;
  std::unique_ptr<SparseMatchModel> sparse_match_;
  std::unique_ptr<SparseBitModel> sparse_bit_;
  std::unique_ptr<SparseModel> sparse_;
  std::unique_ptr<ChartModel> chart_;
  std::unique_ptr<RecordModel> record_;
  std::unique_ptr<CharGroupModel> char_group_;
  std::unique_ptr<TextModel> text_;
  std::unique_ptr<WordModel> word_;
  std::unique_ptr<IndirectModel> indirect_;
  std::unique_ptr<DmcForest> dmc_;
  std::unique_ptr<NestModel> nest_;
  std::unique_ptr<XMLModel> xml_;
  std::unique_ptr<LinearPredictionModel> linear_;
  std::unique_ptr<SimilarityModelPair> similarity_;
  std::unique_ptr<ExeModel> exe_;
  std::unique_ptr<SSE> sse_;
};

template <typename ReadBit>
std::uint8_t process_byte(GenericSseSession &session, ReadBit &&read_bit) {
  std::uint8_t value = 0;
  for (std::uint8_t bit_index = 0; bit_index < 8U; ++bit_index) {
    const std::uint32_t probability = session.predict();
    const std::uint8_t bit = read_bit(probability, bit_index);
    if (bit > 1U) {
      throw std::logic_error(
          "PAQ8px Generic+SSE bit callback returned non-bit");
    }
    session.observe(bit, probability);
    value = static_cast<std::uint8_t>((value << 1U) | bit);
  }
  return value;
}

} // namespace

std::size_t
Paq8pxGenericSseBackend::maximum_payload_size(const std::size_t input_size) {
  if (input_size > (std::numeric_limits<std::size_t>::max() - 64U) / 4U) {
    throw std::length_error(
        "PAQ8px Generic+SSE payload bound overflows size_t");
  }
  return input_size * 4U + 64U;
}

std::vector<std::uint8_t>
Paq8pxGenericSseBackend::encode(const ByteView input) const {
  if (input.size() > std::numeric_limits<std::uint32_t>::max()) {
    throw std::length_error(
        "PAQ8px Generic+SSE input exceeds 32-bit block limit");
  }
  if (!input.empty() && input.data() == nullptr) {
    throw std::invalid_argument("PAQ8px Generic+SSE input has null data");
  }

  GenericSseSession session(input.size());
  std::ostringstream encoded(std::ios::out | std::ios::binary);
  BinaryArithmeticEncoderStream coder(encoded);
  for (std::size_t position = 0; position < input.size(); ++position) {
    const std::uint8_t reconstructed =
        process_byte(session,
                     [&](const std::uint32_t probability,
                         const std::uint8_t bit_index) -> std::uint8_t {
                       const std::uint8_t bit = static_cast<std::uint8_t>(
                           (input[position] >> (7U - bit_index)) & 1U);
                       coder.write_bit(probability, kProbabilityScale, bit);
                       return bit;
                     });
    if (reconstructed != input[position]) {
      throw std::logic_error(
          "PAQ8px Generic+SSE encoder reconstructed wrong byte");
    }
  }
  coder.finish();
  const std::string bytes = encoded.str();
  if (bytes.size() > maximum_payload_size(input.size())) {
    throw std::runtime_error("PAQ8px Generic+SSE payload exceeded safe bound");
  }
  return {reinterpret_cast<const std::uint8_t *>(bytes.data()),
          reinterpret_cast<const std::uint8_t *>(bytes.data()) + bytes.size()};
}

std::vector<std::uint8_t>
Paq8pxGenericSseBackend::decode(const ByteView payload,
                                const std::size_t expected_size) const {
  if (expected_size > std::numeric_limits<std::uint32_t>::max()) {
    throw std::length_error(
        "PAQ8px Generic+SSE output exceeds 32-bit block limit");
  }
  if (payload.size() > maximum_payload_size(expected_size) ||
      (expected_size != 0U && payload.empty())) {
    throw std::invalid_argument(
        "PAQ8px Generic+SSE payload violates size contract");
  }
  if (!payload.empty() && payload.data() == nullptr) {
    throw std::invalid_argument("PAQ8px Generic+SSE payload has null data");
  }

  const std::string bytes =
      payload.empty()
          ? std::string{}
          : std::string(reinterpret_cast<const char *>(payload.data()),
                        payload.size());
  std::istringstream encoded(bytes, std::ios::in | std::ios::binary);
  BinaryArithmeticDecoderStream coder(encoded);
  GenericSseSession session(expected_size);
  std::vector<std::uint8_t> output;
  output.reserve(expected_size);
  for (std::size_t position = 0; position < expected_size; ++position) {
    static_cast<void>(position);
    output.push_back(process_byte(session,
                                  [&](const std::uint32_t probability,
                                      const std::uint8_t) -> std::uint8_t {
                                    return coder.read_bit(probability,
                                                          kProbabilityScale);
                                  }));
  }
  return output;
}

} // namespace hz::r2
