#include "r2/entropy/paq8px_detected_sse_backend.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "r2/entropy/binary_arithmetic_codec.h"
#include "r2/entropy/paq8px_block_detector.h"
#include "r2/entropy/paq8px_generic_sse_backend.h"

#include "MixerFactory.hpp"
#include "Mixer_Scalar.hpp"
#include "SSE.hpp"
#include "Shared.hpp"
#include "model/Audio16BitModel.hpp"
#include "model/Audio8BitModel.hpp"
#include "model/CharGroupModel.hpp"
#include "model/ChartModel.hpp"
#include "model/DecAlphaModel.hpp"
#include "model/DmcForest.hpp"
#include "model/ExeModel.hpp"
#include "model/Image24BitModel.hpp"
#include "model/Image8BitModel.hpp"
#include "model/IndirectModel.hpp"
#include "model/JpegModel.hpp"
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

constexpr std::uint8_t kPayloadVersion = 1;
constexpr std::size_t kProfileHeaderSize = 22;
constexpr std::uint32_t kProbabilityScale = 1U << 24U;
constexpr std::uint32_t kDonorPrecision = 31U;
constexpr int kDonorLevel = 1;
constexpr std::uint32_t kDonorBufferSize = 1U << 20U;

void append_u32(std::vector<std::uint8_t> &output, const std::uint32_t value) {
  for (unsigned shift = 0; shift < 32; shift += 8) {
    output.push_back(static_cast<std::uint8_t>(value >> shift));
  }
}

std::uint32_t read_u32(const ByteView input, const std::size_t offset) {
  if (offset > input.size() || input.size() - offset < 4U) {
    throw std::invalid_argument("PAQ8px detected-SSE truncated profile header");
  }
  std::uint32_t value = 0;
  for (unsigned byte = 0; byte < 4; ++byte) {
    value |= static_cast<std::uint32_t>(input[offset + byte]) << (byte * 8U);
  }
  return value;
}

ByteView slice(const ByteView input, const std::size_t offset,
               const std::size_t length) {
  if (offset > input.size() || length > input.size() - offset) {
    throw std::out_of_range("PAQ8px detected-SSE slice exceeds input");
  }
  return length == 0U ? ByteView{} : ByteView(input.data() + offset, length);
}

class DonorSession {
public:
  DonorSession(const BlockType type, const int info) : factory_(&shared_) {
    shared_.init(kDonorLevel, kDonorBufferSize);
    shared_.chosenSimd = SIMDType::SIMD_NONE;
    shared_.State.blockType = type;
    shared_.State.blockInfo = info;
    shared_.State.blockPos = std::numeric_limits<std::uint32_t>::max();
    shared_.State.Match.expectedByte = 256U;
    sse_ = std::make_unique<SSE>(&shared_);
  }

  virtual ~DonorSession() = default;

  std::uint32_t predict() {
    if (shared_.State.bitPosition == 0U) {
      ++shared_.State.blockPos;
    }
    const int p12 = std::clamp(mix(), 1, 4095);
    const std::uint32_t p31 = sse_->p(static_cast<std::uint32_t>(p12));
    return std::clamp(p31 >> (kDonorPrecision - 24U), 1U,
                      kProbabilityScale - 1U);
  }

  void observe(const std::uint8_t bit, const std::uint32_t p24) {
    if (bit > 1U || p24 == 0U || p24 >= kProbabilityScale) {
      throw std::invalid_argument("PAQ8px detected-SSE invalid bit state");
    }
    const std::uint32_t p31 = p24 << (kDonorPrecision - 24U);
    const bool missed = static_cast<std::uint8_t>(p31 >> 30U) != bit;
    shared_.update(bit, p31, missed);
  }

protected:
  virtual int mix() = 0;

  Shared shared_;
  MixerFactory factory_;

private:
  std::unique_ptr<SSE> sse_;
};

class GenericExeSession final : public DonorSession {
public:
  explicit GenericExeSession(const BlockType type) : DonorSession(type, -1) {
    mixer_ = std::make_unique<Mixer_Scalar>(
        &shared_, kInputs, kContexts, kContextSets, 0);
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
    record_->setParam(0);
    text_->setCmScale(64);
    word_->setCmScale(64);
    word_->setParam(0);
  }

private:
  static constexpr int kInputs =
      1 + MatchModel::MIXERINPUTS + NormalModel::MIXERINPUTS +
      SparseMatchModel::MIXERINPUTS + SparseModel::MIXERINPUTS +
      SparseBitModel::MIXERINPUTS + ChartModel::MIXERINPUTS +
      RecordModel::MIXERINPUTS + CharGroupModel::MIXERINPUTS +
      TextModel::MIXERINPUTS + WordModel::MIXERINPUTS_BIN +
      IndirectModel::MIXERINPUTS + DmcForest::MIXERINPUTS +
      NestModel::MIXERINPUTS + XMLModel::MIXERINPUTS +
      LinearPredictionModel::MIXERINPUTS + 2 * SimilarityModel::MIXERINPUTS +
      ExeModel::MIXERINPUTS;
  static constexpr int kContexts =
      MatchModel::MIXERCONTEXTS + NormalModel::MIXERCONTEXTS_PRE +
      NormalModel::MIXERCONTEXTS_POST + SparseMatchModel::MIXERCONTEXTS +
      SparseModel::MIXERCONTEXTS + SparseBitModel::MIXERCONTEXTS +
      ChartModel::MIXERCONTEXTS + RecordModel::MIXERCONTEXTS +
      CharGroupModel::MIXERCONTEXTS + TextModel::MIXERCONTEXTS +
      WordModel::MIXERCONTEXTS + IndirectModel::MIXERCONTEXTS +
      DmcForest::MIXERCONTEXTS + NestModel::MIXERCONTEXTS +
      XMLModel::MIXERCONTEXTS + LinearPredictionModel::MIXERCONTEXTS +
      2 * SimilarityModel::MIXERCONTEXTS + ExeModel::MIXERCONTEXTS;
  static constexpr int kContextSets =
      MatchModel::MIXERCONTEXTSETS + NormalModel::MIXERCONTEXTSETS_PRE +
      NormalModel::MIXERCONTEXTSETS_POST +
      SparseMatchModel::MIXERCONTEXTSETS + SparseModel::MIXERCONTEXTSETS +
      SparseBitModel::MIXERCONTEXTSETS + ChartModel::MIXERCONTEXTSETS +
      RecordModel::MIXERCONTEXTSETS + CharGroupModel::MIXERCONTEXTSETS +
      TextModel::MIXERCONTEXTSETS + WordModel::MIXERCONTEXTSETS +
      IndirectModel::MIXERCONTEXTSETS + DmcForest::MIXERCONTEXTSETS +
      NestModel::MIXERCONTEXTSETS + XMLModel::MIXERCONTEXTSETS +
      LinearPredictionModel::MIXERCONTEXTSETS +
      2 * SimilarityModel::MIXERCONTEXTSETS + ExeModel::MIXERCONTEXTSETS;

  int mix() override {
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
    return mixer_->p();
  }

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
};

class TextSession final : public DonorSession {
public:
  explicit TextSession(const BlockType type) : DonorSession(type, 0) {
    mixer_ = std::make_unique<Mixer_Scalar>(&shared_, kInputs, kContexts,
                                            kContextSets, 0);
    mixer_->setScaleFactor(940, 60);
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
    record_->setParam(0);
    text_->setCmScale(74);
    word_->setCmScale(74);
    word_->setParam(0);
  }

private:
  static constexpr int kInputs =
      1 + MatchModel::MIXERINPUTS + NormalModel::MIXERINPUTS +
      SparseMatchModel::MIXERINPUTS + SparseModel::MIXERINPUTS_TEXT +
      SparseBitModel::MIXERINPUTS_TEXT + ChartModel::MIXERINPUTS_TEXT +
      RecordModel::MIXERINPUTS + CharGroupModel::MIXERINPUTS +
      TextModel::MIXERINPUTS + WordModel::MIXERINPUTS_TEXT +
      IndirectModel::MIXERINPUTS_TEXT + DmcForest::MIXERINPUTS +
      NestModel::MIXERINPUTS + XMLModel::MIXERINPUTS;
  static constexpr int kContexts =
      MatchModel::MIXERCONTEXTS + NormalModel::MIXERCONTEXTS_PRE +
      NormalModel::MIXERCONTEXTS_POST + SparseMatchModel::MIXERCONTEXTS +
      SparseModel::MIXERCONTEXTS_TEXT + SparseBitModel::MIXERCONTEXTS +
      ChartModel::MIXERCONTEXTS + RecordModel::MIXERCONTEXTS +
      CharGroupModel::MIXERCONTEXTS + TextModel::MIXERCONTEXTS +
      WordModel::MIXERCONTEXTS + IndirectModel::MIXERCONTEXTS +
      DmcForest::MIXERCONTEXTS + NestModel::MIXERCONTEXTS +
      XMLModel::MIXERCONTEXTS;
  static constexpr int kContextSets =
      MatchModel::MIXERCONTEXTSETS + NormalModel::MIXERCONTEXTSETS_PRE +
      NormalModel::MIXERCONTEXTSETS_POST +
      SparseMatchModel::MIXERCONTEXTSETS +
      SparseModel::MIXERCONTEXTSETS_TEXT +
      SparseBitModel::MIXERCONTEXTSETS + ChartModel::MIXERCONTEXTSETS +
      RecordModel::MIXERCONTEXTSETS + CharGroupModel::MIXERCONTEXTSETS +
      TextModel::MIXERCONTEXTSETS + WordModel::MIXERCONTEXTSETS +
      IndirectModel::MIXERCONTEXTSETS + DmcForest::MIXERCONTEXTSETS +
      NestModel::MIXERCONTEXTSETS + XMLModel::MIXERCONTEXTSETS;

  int mix() override {
    mixer_->add(256);
    normal_->mix(*mixer_);
    normal_->mixPost(*mixer_);
    match_->mix(*mixer_);
    sparse_match_->mix(*mixer_);
    sparse_bit_->mix(*mixer_);
    sparse_->mix(*mixer_);
    xml_->mix(*mixer_);
    nest_->mix(*mixer_);
    chart_->mix(*mixer_);
    record_->mix(*mixer_);
    char_group_->mix(*mixer_);
    text_->mix(*mixer_);
    word_->mix(*mixer_);
    indirect_->mix(*mixer_);
    dmc_->mix(*mixer_);
    return mixer_->p();
  }

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
};

class Image8Session final : public DonorSession {
public:
  Image8Session(const BlockType type, const int width)
      : DonorSession(type, width) {
    if (width <= 0) {
      throw std::invalid_argument("PAQ8px Image8 width must be positive");
    }
    mixer_ = std::make_unique<Mixer_Scalar>(
        &shared_, 1 + MatchModel::MIXERINPUTS + NormalModel::MIXERINPUTS +
                      Image8BitModel::MIXERINPUTS,
        MatchModel::MIXERCONTEXTS + NormalModel::MIXERCONTEXTS_PRE +
            Image8BitModel::MIXERCONTEXTS,
        MatchModel::MIXERCONTEXTSETS + NormalModel::MIXERCONTEXTSETS_PRE +
            Image8BitModel::MIXERCONTEXTSETS,
        0);
    normal_ = std::make_unique<NormalModel>(&shared_, shared_.mem * 32U);
    match_ =
        std::make_unique<MatchModel>(&shared_, shared_.mem / 4U, shared_.mem);
    image_ = std::make_unique<Image8BitModel>(&shared_, shared_.mem * 4U);
    const bool gray = type == BlockType::IMAGE8GRAY;
    image_->setParam(width, gray ? 1U : 0U);
    mixer_->setScaleFactor(gray ? 1300 : 1600, gray ? 100 : 110);
  }

private:
  int mix() override {
    mixer_->add(256);
    normal_->mix(*mixer_);
    match_->mix(*mixer_);
    image_->mix(*mixer_);
    return mixer_->p();
  }

  std::unique_ptr<Mixer_Scalar> mixer_;
  std::unique_ptr<NormalModel> normal_;
  std::unique_ptr<MatchModel> match_;
  std::unique_ptr<Image8BitModel> image_;
};

class Image24Session final : public DonorSession {
public:
  Image24Session(const BlockType type, const int width)
      : DonorSession(type, width) {
    if (width <= 0) {
      throw std::invalid_argument("PAQ8px Image24 width must be positive");
    }
    constexpr int inputs = MatchModel::MIXERINPUTS +
                           NormalModel::MIXERINPUTS +
                           Image24BitModel::MIXERINPUTS;
    constexpr int contexts = MatchModel::MIXERCONTEXTS +
                             NormalModel::MIXERCONTEXTS_PRE +
                             Image24BitModel::MIXERCONTEXTS;
    constexpr int sets = MatchModel::MIXERCONTEXTSETS +
                         NormalModel::MIXERCONTEXTSETS_PRE +
                         Image24BitModel::MIXERCONTEXTSETS;
    for (std::size_t index = 0; index < mixers_.size(); ++index) {
      mixers_[index] =
          std::make_unique<Mixer_Scalar>(&shared_, inputs, contexts, sets, 0);
    }
    mixers_[0]->setScaleFactor(490, 130);
    mixers_[1]->setScaleFactor(620, 135);
    mixers_[2]->setScaleFactor(770, 140);
    for (auto &mixer : mixers_) {
      mixer->setLowerLimitOfLearningRate(5, 1);
    }
    normal_ = std::make_unique<NormalModel>(&shared_, shared_.mem * 32U);
    match_ =
        std::make_unique<MatchModel>(&shared_, shared_.mem / 4U, shared_.mem);
    image_ = std::make_unique<Image24BitModel>(&shared_, shared_.mem * 4U);
    image_->setParam(width, type == BlockType::IMAGE32 ? 1 : 0);
  }

private:
  int mix() override {
    image_->update();
    const int color = image_->color;
    Mixer_Scalar &mixer = *mixers_[color < 0 ? 0U
                                             : std::min<std::size_t>(
                                                   static_cast<std::size_t>(color),
                                                   mixers_.size() - 1U)];
    normal_->mix(mixer);
    match_->mix(mixer);
    image_->mix(mixer);
    return mixer.p();
  }

  std::array<std::unique_ptr<Mixer_Scalar>, 3> mixers_{};
  std::unique_ptr<NormalModel> normal_;
  std::unique_ptr<MatchModel> match_;
  std::unique_ptr<Image24BitModel> image_;
};

class Audio8Session final : public DonorSession {
public:
  Audio8Session(const BlockType type, const int info)
      : DonorSession(type, info) {
    mixer_ = std::make_unique<Mixer_Scalar>(
        &shared_, 1 + MatchModel::MIXERINPUTS + NormalModel::MIXERINPUTS +
                      Audio8BitModel::MIXERINPUTS + RecordModel::MIXERINPUTS,
        MatchModel::MIXERCONTEXTS + NormalModel::MIXERCONTEXTS_PRE +
            Audio8BitModel::MIXERCONTEXTS + RecordModel::MIXERCONTEXTS,
        MatchModel::MIXERCONTEXTSETS + NormalModel::MIXERCONTEXTSETS_PRE +
            Audio8BitModel::MIXERCONTEXTSETS + RecordModel::MIXERCONTEXTSETS,
        0);
    mixer_->setScaleFactor(850, 140);
    normal_ = std::make_unique<NormalModel>(&shared_, shared_.mem * 32U);
    match_ =
        std::make_unique<MatchModel>(&shared_, shared_.mem / 4U, shared_.mem);
    audio_ = std::make_unique<Audio8BitModel>(&shared_);
    record_ = std::make_unique<RecordModel>(&shared_, shared_.mem * 2U);
    audio_->setParam(info);
    record_->setParam(static_cast<std::uint32_t>(audio_->stereo + 1));
  }

private:
  int mix() override {
    mixer_->add(256);
    normal_->mix(*mixer_);
    match_->mix(*mixer_);
    audio_->mix(*mixer_);
    record_->mix(*mixer_);
    return mixer_->p();
  }

  std::unique_ptr<Mixer_Scalar> mixer_;
  std::unique_ptr<NormalModel> normal_;
  std::unique_ptr<MatchModel> match_;
  std::unique_ptr<Audio8BitModel> audio_;
  std::unique_ptr<RecordModel> record_;
};

class Audio16Session final : public DonorSession {
public:
  Audio16Session(const BlockType type, const int info)
      : DonorSession(type, info) {
    mixer_ = std::make_unique<Mixer_Scalar>(
        &shared_, 1 + MatchModel::MIXERINPUTS + NormalModel::MIXERINPUTS +
                      Audio16BitModel::MIXERINPUTS + RecordModel::MIXERINPUTS,
        MatchModel::MIXERCONTEXTS + NormalModel::MIXERCONTEXTS_PRE +
            Audio16BitModel::MIXERCONTEXTS + RecordModel::MIXERCONTEXTS,
        MatchModel::MIXERCONTEXTSETS + NormalModel::MIXERCONTEXTSETS_PRE +
            Audio16BitModel::MIXERCONTEXTSETS + RecordModel::MIXERCONTEXTSETS,
        0);
    mixer_->setScaleFactor(1024, 128);
    normal_ = std::make_unique<NormalModel>(&shared_, shared_.mem * 32U);
    match_ =
        std::make_unique<MatchModel>(&shared_, shared_.mem / 4U, shared_.mem);
    audio_ = std::make_unique<Audio16BitModel>(&shared_);
    record_ = std::make_unique<RecordModel>(&shared_, shared_.mem * 2U);
    audio_->setParam(info);
    record_->setParam(static_cast<std::uint32_t>((audio_->stereo + 1) * 2));
  }

private:
  int mix() override {
    mixer_->add(256);
    normal_->mix(*mixer_);
    match_->mix(*mixer_);
    audio_->mix(*mixer_);
    record_->mix(*mixer_);
    return mixer_->p();
  }

  std::unique_ptr<Mixer_Scalar> mixer_;
  std::unique_ptr<NormalModel> normal_;
  std::unique_ptr<MatchModel> match_;
  std::unique_ptr<Audio16BitModel> audio_;
  std::unique_ptr<RecordModel> record_;
};

class JpegSession final : public DonorSession {
public:
  JpegSession() : DonorSession(BlockType::JPEG, -1) {
    mixer_ = std::make_unique<Mixer_Scalar>(
        &shared_,
        1 + MatchModel::MIXERINPUTS + NormalModel::MIXERINPUTS +
            JpegModel::MIXERINPUTS + SparseMatchModel::MIXERINPUTS +
            SparseModel::MIXERINPUTS + SparseBitModel::MIXERINPUTS +
            RecordModel::MIXERINPUTS + CharGroupModel::MIXERINPUTS +
            TextModel::MIXERINPUTS + WordModel::MIXERINPUTS_BIN +
            LinearPredictionModel::MIXERINPUTS,
        MatchModel::MIXERCONTEXTS + NormalModel::MIXERCONTEXTS_PRE +
            JpegModel::MIXERCONTEXTS + SparseMatchModel::MIXERCONTEXTS +
            SparseModel::MIXERCONTEXTS + SparseBitModel::MIXERCONTEXTS +
            RecordModel::MIXERCONTEXTS + CharGroupModel::MIXERCONTEXTS +
            TextModel::MIXERCONTEXTS + WordModel::MIXERCONTEXTS +
            LinearPredictionModel::MIXERCONTEXTS,
        MatchModel::MIXERCONTEXTSETS + NormalModel::MIXERCONTEXTSETS_PRE +
            JpegModel::MIXERCONTEXTSETS + SparseMatchModel::MIXERCONTEXTSETS +
            SparseModel::MIXERCONTEXTSETS +
            SparseBitModel::MIXERCONTEXTSETS + RecordModel::MIXERCONTEXTSETS +
            CharGroupModel::MIXERCONTEXTSETS + TextModel::MIXERCONTEXTSETS +
            WordModel::MIXERCONTEXTSETS +
            LinearPredictionModel::MIXERCONTEXTSETS,
        0);
    normal_ = std::make_unique<NormalModel>(&shared_, shared_.mem * 32U);
    match_ =
        std::make_unique<MatchModel>(&shared_, shared_.mem / 4U, shared_.mem);
    jpeg_ = std::make_unique<JpegModel>(&shared_, &factory_, shared_.mem);
    sparse_match_ = std::make_unique<SparseMatchModel>(&shared_, shared_.mem);
    sparse_bit_ = std::make_unique<SparseBitModel>(&shared_, shared_.mem / 4U);
    sparse_ = std::make_unique<SparseModel>(&shared_, shared_.mem * 4U);
    record_ = std::make_unique<RecordModel>(&shared_, shared_.mem * 2U);
    char_group_ = std::make_unique<CharGroupModel>(&shared_, shared_.mem / 2U);
    text_ = std::make_unique<TextModel>(&shared_, shared_.mem * 16U);
    word_ = std::make_unique<WordModel>(&shared_, shared_.mem * 16U);
    linear_ = std::make_unique<LinearPredictionModel>(&shared_);
    record_->setParam(0);
    text_->setCmScale(64);
    word_->setCmScale(64);
    word_->setParam(0);
  }

private:
  int mix() override {
    mixer_->add(256);
    normal_->mix(*mixer_);
    match_->mix(*mixer_);
    if (jpeg_->mix(*mixer_) != 0) {
      mixer_->setScaleFactor(1024, 256);
      return mixer_->p();
    }
    sparse_match_->mix(*mixer_);
    sparse_bit_->mix(*mixer_);
    sparse_->mix(*mixer_);
    record_->mix(*mixer_);
    char_group_->mix(*mixer_);
    text_->mix(*mixer_);
    word_->mix(*mixer_);
    linear_->mix(*mixer_);
    mixer_->setScaleFactor(1200, 120);
    return mixer_->p();
  }

  std::unique_ptr<Mixer_Scalar> mixer_;
  std::unique_ptr<NormalModel> normal_;
  std::unique_ptr<MatchModel> match_;
  std::unique_ptr<JpegModel> jpeg_;
  std::unique_ptr<SparseMatchModel> sparse_match_;
  std::unique_ptr<SparseBitModel> sparse_bit_;
  std::unique_ptr<SparseModel> sparse_;
  std::unique_ptr<RecordModel> record_;
  std::unique_ptr<CharGroupModel> char_group_;
  std::unique_ptr<TextModel> text_;
  std::unique_ptr<WordModel> word_;
  std::unique_ptr<LinearPredictionModel> linear_;
};

class DecSession final : public DonorSession {
public:
  explicit DecSession(const int info)
      : DonorSession(BlockType::DEC_ALPHA, info) {
    mixer_ = std::make_unique<Mixer_Scalar>(
        &shared_,
        1 + MatchModel::MIXERINPUTS + NormalModel::MIXERINPUTS +
            SparseMatchModel::MIXERINPUTS + SparseModel::MIXERINPUTS +
            SparseBitModel::MIXERINPUTS + ChartModel::MIXERINPUTS +
            RecordModel::MIXERINPUTS + TextModel::MIXERINPUTS +
            WordModel::MIXERINPUTS_BIN + IndirectModel::MIXERINPUTS +
            ExeModel::MIXERINPUTS + DECAlphaModel::MIXERINPUTS,
        MatchModel::MIXERCONTEXTS + NormalModel::MIXERCONTEXTS_PRE +
            NormalModel::MIXERCONTEXTS_POST +
            SparseMatchModel::MIXERCONTEXTS + SparseModel::MIXERCONTEXTS +
            SparseBitModel::MIXERCONTEXTS + ChartModel::MIXERCONTEXTS +
            RecordModel::MIXERCONTEXTS + TextModel::MIXERCONTEXTS +
            WordModel::MIXERCONTEXTS + IndirectModel::MIXERCONTEXTS +
            DECAlphaModel::MIXERCONTEXTS + ExeModel::MIXERCONTEXTS,
        MatchModel::MIXERCONTEXTSETS + NormalModel::MIXERCONTEXTSETS_PRE +
            NormalModel::MIXERCONTEXTSETS_POST +
            SparseMatchModel::MIXERCONTEXTSETS +
            SparseModel::MIXERCONTEXTSETS +
            SparseBitModel::MIXERCONTEXTSETS + ChartModel::MIXERCONTEXTSETS +
            RecordModel::MIXERCONTEXTSETS + TextModel::MIXERCONTEXTSETS +
            WordModel::MIXERCONTEXTSETS + IndirectModel::MIXERCONTEXTSETS +
            DECAlphaModel::MIXERCONTEXTSETS + ExeModel::MIXERCONTEXTSETS,
        0);
    mixer_->setScaleFactor(1800, 60);
    normal_ = std::make_unique<NormalModel>(&shared_, shared_.mem * 32U);
    match_ =
        std::make_unique<MatchModel>(&shared_, shared_.mem / 4U, shared_.mem);
    sparse_match_ = std::make_unique<SparseMatchModel>(&shared_, shared_.mem);
    sparse_bit_ = std::make_unique<SparseBitModel>(&shared_, shared_.mem / 4U);
    sparse_ = std::make_unique<SparseModel>(&shared_, shared_.mem * 4U);
    chart_ = std::make_unique<ChartModel>(&shared_, shared_.mem * 4U);
    record_ = std::make_unique<RecordModel>(&shared_, shared_.mem * 2U);
    text_ = std::make_unique<TextModel>(&shared_, shared_.mem * 16U);
    word_ = std::make_unique<WordModel>(&shared_, shared_.mem * 16U);
    indirect_ = std::make_unique<IndirectModel>(&shared_, shared_.mem * 2U);
    dec_ = std::make_unique<DECAlphaModel>(&shared_);
    exe_ = std::make_unique<ExeModel>(&shared_, shared_.mem * 4U);
    record_->setParam(16);
    text_->setCmScale(64);
    word_->setCmScale(64);
    word_->setParam(16);
  }

private:
  int mix() override {
    mixer_->add(256);
    normal_->mix(*mixer_);
    normal_->mixPost(*mixer_);
    match_->mix(*mixer_);
    sparse_match_->mix(*mixer_);
    sparse_bit_->mix(*mixer_);
    sparse_->mix(*mixer_);
    chart_->mix(*mixer_);
    record_->mix(*mixer_);
    text_->mix(*mixer_);
    word_->mix(*mixer_);
    indirect_->mix(*mixer_);
    dec_->mix(*mixer_);
    exe_->mix(*mixer_);
    return mixer_->p();
  }

  std::unique_ptr<Mixer_Scalar> mixer_;
  std::unique_ptr<NormalModel> normal_;
  std::unique_ptr<MatchModel> match_;
  std::unique_ptr<SparseMatchModel> sparse_match_;
  std::unique_ptr<SparseBitModel> sparse_bit_;
  std::unique_ptr<SparseModel> sparse_;
  std::unique_ptr<ChartModel> chart_;
  std::unique_ptr<RecordModel> record_;
  std::unique_ptr<TextModel> text_;
  std::unique_ptr<WordModel> word_;
  std::unique_ptr<IndirectModel> indirect_;
  std::unique_ptr<DECAlphaModel> dec_;
  std::unique_ptr<ExeModel> exe_;
};

std::unique_ptr<DonorSession> make_session(const BlockType type,
                                           const int info) {
  switch (type) {
  case BlockType::TEXT:
  case BlockType::TEXT_EOL:
    return std::make_unique<TextSession>(type);
  case BlockType::IMAGE8:
  case BlockType::IMAGE8GRAY:
    return std::make_unique<Image8Session>(type, info & 0xFFFFFF);
  case BlockType::IMAGE24:
  case BlockType::IMAGE32:
    return std::make_unique<Image24Session>(type, info & 0xFFFFFF);
  case BlockType::AUDIO:
  case BlockType::AUDIO_LE:
    return (info & 2) == 0
               ? std::unique_ptr<DonorSession>(
                     std::make_unique<Audio8Session>(type, info))
               : std::unique_ptr<DonorSession>(
                     std::make_unique<Audio16Session>(type, info));
  case BlockType::JPEG:
    return std::make_unique<JpegSession>();
  case BlockType::DEC_ALPHA:
    return std::make_unique<DecSession>(info);
  case BlockType::EXE:
    return std::make_unique<GenericExeSession>(type);
  default:
    throw std::invalid_argument("unsupported PAQ8px specialist block type");
  }
}

template <typename ReadBit>
std::uint8_t process_byte(DonorSession &session, ReadBit &&read_bit) {
  std::uint8_t value = 0;
  for (std::uint8_t bit_index = 0; bit_index < 8U; ++bit_index) {
    const std::uint32_t probability = session.predict();
    const std::uint8_t bit = read_bit(probability, bit_index);
    if (bit > 1U) {
      throw std::logic_error("PAQ8px specialist callback returned non-bit");
    }
    session.observe(bit, probability);
    value = static_cast<std::uint8_t>((value << 1U) | bit);
  }
  return value;
}

std::vector<std::uint8_t> encode_specialist(const ByteView input,
                                            const BlockType type,
                                            const int info) {
  auto session = make_session(type, info);
  std::ostringstream encoded(std::ios::out | std::ios::binary);
  BinaryArithmeticEncoderStream coder(encoded);
  for (std::size_t position = 0; position < input.size(); ++position) {
    const std::uint8_t reconstructed = process_byte(
        *session, [&](const std::uint32_t probability,
                      const std::uint8_t bit_index) -> std::uint8_t {
          const std::uint8_t bit = static_cast<std::uint8_t>(
              (input[position] >> (7U - bit_index)) & 1U);
          coder.write_bit(probability, kProbabilityScale, bit);
          return bit;
        });
    if (reconstructed != input[position]) {
      throw std::logic_error("PAQ8px specialist reconstructed wrong byte");
    }
  }
  coder.finish();
  const std::string bytes = encoded.str();
  return {reinterpret_cast<const std::uint8_t *>(bytes.data()),
          reinterpret_cast<const std::uint8_t *>(bytes.data()) + bytes.size()};
}

std::vector<std::uint8_t> decode_specialist(const ByteView payload,
                                            const std::size_t expected_size,
                                            const BlockType type,
                                            const int info) {
  if (expected_size != 0U && payload.empty()) {
    throw std::invalid_argument("empty PAQ8px specialist payload");
  }
  const std::string bytes =
      payload.empty()
          ? std::string{}
          : std::string(reinterpret_cast<const char *>(payload.data()),
                        payload.size());
  std::istringstream encoded(bytes, std::ios::in | std::ios::binary);
  BinaryArithmeticDecoderStream coder(encoded);
  auto session = make_session(type, info);
  std::vector<std::uint8_t> output;
  output.reserve(expected_size);
  for (std::size_t position = 0; position < expected_size; ++position) {
    static_cast<void>(position);
    output.push_back(process_byte(
        *session, [&](const std::uint32_t probability,
                      const std::uint8_t) -> std::uint8_t {
          return coder.read_bit(probability, kProbabilityScale);
        }));
  }
  return output;
}

std::vector<std::uint8_t> encode_generic(const ByteView input) {
  return input.empty() ? std::vector<std::uint8_t>{}
                       : Paq8pxGenericSseBackend().encode(input);
}

std::vector<std::uint8_t> decode_generic(const ByteView payload,
                                         const std::size_t expected_size) {
  return expected_size == 0U
             ? std::vector<std::uint8_t>{}
             : Paq8pxGenericSseBackend().decode(payload, expected_size);
}

bool valid_type(const BlockType type) noexcept {
  switch (type) {
  case BlockType::TEXT:
  case BlockType::TEXT_EOL:
  case BlockType::IMAGE8:
  case BlockType::IMAGE8GRAY:
  case BlockType::IMAGE24:
  case BlockType::IMAGE32:
  case BlockType::AUDIO:
  case BlockType::AUDIO_LE:
  case BlockType::JPEG:
  case BlockType::EXE:
  case BlockType::DEC_ALPHA:
    return true;
  default:
    return false;
  }
}

} // namespace

std::size_t Paq8pxDetectedSseBackend::maximum_payload_size(
    const std::size_t input_size) {
  constexpr std::size_t overhead = kProfileHeaderSize + 3U * 64U;
  if (input_size >
      (std::numeric_limits<std::size_t>::max() - overhead) / 4U) {
    throw std::length_error("PAQ8px detected-SSE payload bound overflow");
  }
  return input_size * 4U + overhead;
}

std::vector<std::uint8_t>
Paq8pxDetectedSseBackend::encode(const ByteView input) const {
  if (input.size() > std::numeric_limits<std::uint32_t>::max()) {
    throw std::length_error("PAQ8px detected-SSE input exceeds 32-bit size");
  }
  if (!input.empty() && input.data() == nullptr) {
    throw std::invalid_argument("PAQ8px detected-SSE input has null data");
  }

  Paq8pxBlockProfile profile = detect_paq8px_block_profile(input);
  const bool specialized = paq8px_profile_uses_specialist(profile);
  if (!specialized) {
    profile = {};
    profile.data_start = static_cast<std::uint32_t>(input.size());
  }
  const std::size_t start = profile.data_start;
  const std::size_t length = profile.data_length;
  if (start > input.size() || length > input.size() - start) {
    throw std::logic_error("PAQ8px detector returned invalid segment range");
  }

  const ByteView prefix = slice(input, 0, start);
  const ByteView specialist = slice(input, start, length);
  const ByteView suffix =
      slice(input, start + length, input.size() - start - length);
  const BlockType type = static_cast<BlockType>(profile.donor_type);

  std::vector<std::uint8_t> prefix_payload = encode_generic(prefix);
  std::vector<std::uint8_t> specialist_payload =
      specialized ? encode_specialist(specialist, type, profile.block_info)
                  : std::vector<std::uint8_t>{};
  std::vector<std::uint8_t> suffix_payload = encode_generic(suffix);

  if (prefix_payload.size() > std::numeric_limits<std::uint32_t>::max() ||
      specialist_payload.size() >
          std::numeric_limits<std::uint32_t>::max()) {
    throw std::length_error("PAQ8px detected-SSE segment payload too large");
  }

  std::vector<std::uint8_t> output;
  output.reserve(kProfileHeaderSize + prefix_payload.size() +
                 specialist_payload.size() + suffix_payload.size());
  output.push_back(kPayloadVersion);
  output.push_back(profile.donor_type);
  append_u32(output, static_cast<std::uint32_t>(profile.block_info));
  append_u32(output, profile.data_start);
  append_u32(output, profile.data_length);
  append_u32(output, static_cast<std::uint32_t>(prefix_payload.size()));
  append_u32(output, static_cast<std::uint32_t>(specialist_payload.size()));
  output.insert(output.end(), prefix_payload.begin(), prefix_payload.end());
  output.insert(output.end(), specialist_payload.begin(),
                specialist_payload.end());
  output.insert(output.end(), suffix_payload.begin(), suffix_payload.end());
  if (output.size() > maximum_payload_size(input.size())) {
    throw std::runtime_error("PAQ8px detected-SSE payload exceeded bound");
  }
  return output;
}

std::vector<std::uint8_t>
Paq8pxDetectedSseBackend::decode(const ByteView payload,
                                 const std::size_t expected_size) const {
  if (expected_size > std::numeric_limits<std::uint32_t>::max()) {
    throw std::length_error("PAQ8px detected-SSE output exceeds 32-bit size");
  }
  if (payload.size() < kProfileHeaderSize ||
      payload.size() > maximum_payload_size(expected_size) ||
      payload.data() == nullptr) {
    throw std::invalid_argument("PAQ8px detected-SSE invalid payload size");
  }
  if (payload[0] != kPayloadVersion) {
    throw std::invalid_argument("PAQ8px detected-SSE unsupported payload version");
  }

  const BlockType type = static_cast<BlockType>(payload[1]);
  const std::int32_t info = static_cast<std::int32_t>(read_u32(payload, 2));
  const std::size_t start = read_u32(payload, 6);
  const std::size_t length = read_u32(payload, 10);
  const std::size_t prefix_payload_size = read_u32(payload, 14);
  const std::size_t specialist_payload_size = read_u32(payload, 18);
  if (start > expected_size || length > expected_size - start ||
      prefix_payload_size > payload.size() - kProfileHeaderSize ||
      specialist_payload_size >
          payload.size() - kProfileHeaderSize - prefix_payload_size) {
    throw std::invalid_argument("PAQ8px detected-SSE invalid segment table");
  }
  const bool specialized = valid_type(type);
  if ((!specialized &&
       (type != BlockType::DEFAULT || length != 0U || start != expected_size)) ||
      (specialized && length == 0U)) {
    throw std::invalid_argument("PAQ8px detected-SSE invalid profile type");
  }

  const std::size_t suffix_payload_offset =
      kProfileHeaderSize + prefix_payload_size + specialist_payload_size;
  const std::size_t suffix_payload_size = payload.size() - suffix_payload_offset;
  const std::size_t suffix_size = expected_size - start - length;
  if ((start == 0U) != (prefix_payload_size == 0U) ||
      (length == 0U) != (specialist_payload_size == 0U) ||
      (suffix_size == 0U) != (suffix_payload_size == 0U)) {
    throw std::invalid_argument("PAQ8px detected-SSE empty segment mismatch");
  }

  std::vector<std::uint8_t> prefix =
      decode_generic(slice(payload, kProfileHeaderSize, prefix_payload_size),
                     start);
  std::vector<std::uint8_t> specialist =
      specialized
          ? decode_specialist(
                slice(payload, kProfileHeaderSize + prefix_payload_size,
                      specialist_payload_size),
                length, type, info)
          : std::vector<std::uint8_t>{};
  std::vector<std::uint8_t> suffix =
      decode_generic(slice(payload, suffix_payload_offset, suffix_payload_size),
                     suffix_size);

  std::vector<std::uint8_t> output;
  output.reserve(expected_size);
  output.insert(output.end(), prefix.begin(), prefix.end());
  output.insert(output.end(), specialist.begin(), specialist.end());
  output.insert(output.end(), suffix.begin(), suffix.end());
  if (output.size() != expected_size) {
    throw std::runtime_error("PAQ8px detected-SSE decoded size mismatch");
  }
  return output;
}

} // namespace hz::r2
