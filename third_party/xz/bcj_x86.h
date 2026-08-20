#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t hz_xz_bcj_x86_encode(uint32_t start_offset, uint8_t *buffer, size_t size);
size_t hz_xz_bcj_x86_decode(uint32_t start_offset, uint8_t *buffer, size_t size);

#ifdef __cplusplus
}
#endif
