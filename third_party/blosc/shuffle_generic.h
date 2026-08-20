#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
void hz_blosc_shuffle_generic(uint32_t type_size, size_t block_size, const uint8_t *source, uint8_t *destination);
void hz_blosc_unshuffle_generic(uint32_t type_size, size_t block_size, const uint8_t *source, uint8_t *destination);
#ifdef __cplusplus
}
#endif
