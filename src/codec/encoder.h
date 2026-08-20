#pragma once

#include <filesystem>

namespace hz {

void compress_file(const std::filesystem::path& input,
                   const std::filesystem::path& output);

}  // namespace hz
