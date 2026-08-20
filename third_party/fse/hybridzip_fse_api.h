#pragma once

#include <cstddef>

// HybridZip compiles the unmodified FSE donor with prefixed public symbols so
// it can coexist with zstd's private FSE copy in one executable.
extern "C" {

std::size_t HZFSE_compressBound(std::size_t size);
std::size_t HZFSE_compress(void* destination,
                           std::size_t destination_capacity,
                           const void* source,
                           std::size_t source_size);
std::size_t HZFSE_decompress(void* destination,
                             std::size_t destination_capacity,
                             const void* source,
                             std::size_t source_size);
unsigned HZFSE_isError(std::size_t code);
const char* HZFSE_getErrorName(std::size_t code);

}
