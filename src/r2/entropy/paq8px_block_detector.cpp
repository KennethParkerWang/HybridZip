#include "r2/entropy/paq8px_block_detector.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>
#include <stdexcept>

#include "../../../third_party/paq8px/block_detection/FiltersDetection.hpp"

namespace hz::r2 {
namespace {

class MemoryFile final : public File {
public:
  explicit MemoryFile(const ByteView input) : input_(input) {}

  bool open(const char *, bool) override { return false; }
  void create(const char *) override {
    throw std::logic_error("PAQ8px detector memory file is read-only");
  }
  void close() override {}

  int getchar() override {
    if (position_ >= input_.size()) {
      return EOF;
    }
    return input_[position_++];
  }

  void putChar(const std::uint8_t) override {
    throw std::logic_error("PAQ8px detector memory file is read-only");
  }

  std::uint64_t blockRead(std::uint8_t *const output,
                          const std::uint64_t count) override {
    if (output == nullptr && count != 0U) {
      throw std::invalid_argument("PAQ8px detector received null output");
    }
    const std::uint64_t available = input_.size() - position_;
    const std::uint64_t copied = std::min(count, available);
    if (copied != 0U) {
      std::memcpy(output, input_.data() + position_,
                  static_cast<std::size_t>(copied));
      position_ += copied;
    }
    return copied;
  }

  void blockWrite(std::uint8_t *, std::uint64_t) override {
    throw std::logic_error("PAQ8px detector memory file is read-only");
  }

  void setpos(const std::uint64_t position) override {
    if (position > input_.size()) {
      throw std::out_of_range("PAQ8px detector seek exceeds input");
    }
    position_ = position;
  }

  void setEnd() override {
    throw std::logic_error("PAQ8px detector memory file is read-only");
  }

  std::uint64_t curPos() override { return position_; }
  bool eof() override { return position_ >= input_.size(); }

private:
  ByteView input_;
  std::uint64_t position_ = 0;
};

bool is_supported_specialist(const BlockType type) noexcept {
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

Paq8pxBlockProfile make_profile(const BlockType type, const int info,
                                const std::uint64_t start,
                                const std::uint64_t length,
                                const std::size_t input_size) {
  if (!is_supported_specialist(type) || length == 0U || start > input_size ||
      length > input_size - start ||
      start > std::numeric_limits<std::uint32_t>::max() ||
      length > std::numeric_limits<std::uint32_t>::max()) {
    return {};
  }
  return {static_cast<std::uint8_t>(type), static_cast<std::int32_t>(info),
          static_cast<std::uint32_t>(start),
          static_cast<std::uint32_t>(length)};
}

} // namespace

Paq8pxBlockProfile detect_paq8px_block_profile(const ByteView input) {
  if (input.size() > std::numeric_limits<std::uint32_t>::max()) {
    throw std::length_error("PAQ8px detector input exceeds 32-bit size");
  }
  if (!input.empty() && input.data() == nullptr) {
    throw std::invalid_argument("PAQ8px detector input has null data");
  }
  if (input.empty()) {
    return {};
  }

  try {
    MemoryFile file(input);
    const TransformOptions options{};
    const DetectionInfo detected = detect(&file, input.size(), &options);
    const Paq8pxBlockProfile special =
        make_profile(detected.Type, detected.DataInfo, detected.DataStart,
                     detected.DataLength, input.size());
    if (paq8px_profile_uses_specialist(special)) {
      return special;
    }

    file.setpos(0);
    const TextDetectionInfo text = detectText(&file, 0, input.size());
    return make_profile(text.Type, 0, text.DataStart, text.DataLength,
                        input.size());
  } catch (const IntentionalException &) {
    return {};
  } catch (const std::out_of_range &) {
    return {};
  }
}

bool paq8px_profile_uses_specialist(
    const Paq8pxBlockProfile &profile) noexcept {
  return is_supported_specialist(static_cast<BlockType>(profile.donor_type)) &&
         profile.data_length != 0U;
}

} // namespace hz::r2
