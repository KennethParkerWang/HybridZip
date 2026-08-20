#include "core/byte_history.h"

#include <stdexcept>

namespace hz {

ByteHistory::ByteHistory(const std::size_t capacity) : storage_(capacity) {
    if (capacity == 0) {
        throw std::invalid_argument("ByteHistory capacity must be positive");
    }
}

std::uint64_t ByteHistory::position() const noexcept {
    return position_;
}

std::size_t ByteHistory::size() const noexcept {
    return size_;
}

std::size_t ByteHistory::capacity() const noexcept {
    return storage_.size();
}

std::uint8_t ByteHistory::back(const std::size_t distance) const {
    if (distance == 0 || distance > size_) {
        throw std::out_of_range("ByteHistory distance is unavailable");
    }
    return at_absolute(position_ - distance);
}

std::uint8_t ByteHistory::at_absolute(const std::uint64_t position) const {
    if (!contains(position)) {
        throw std::out_of_range("ByteHistory position is unavailable");
    }
    return storage_[static_cast<std::size_t>(position % storage_.size())];
}

bool ByteHistory::contains(const std::uint64_t position) const noexcept {
    return position < position_ && position >= position_ - size_;
}

void ByteHistory::push(const std::uint8_t byte) {
    storage_[static_cast<std::size_t>(position_ % storage_.size())] = byte;
    ++position_;
    if (size_ < storage_.size()) {
        ++size_;
    }
}

void ByteHistory::reset() noexcept {
    position_ = 0;
    size_ = 0;
}

}  // namespace hz
