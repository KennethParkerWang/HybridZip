// SPDX-License-Identifier: 0BSD
// Extracted from XZ Utils src/liblzma/simple/x86.c at commit
// 11334a5d4d5ea3e8b2a3cbce74c1062d25cef772.

#include "bcj_x86.h"

#define Test86MSByte(b) ((b) == 0 || (b) == 0xFF)

typedef struct {
    uint32_t prev_mask;
    uint32_t prev_pos;
} hz_xz_simple_x86;

static size_t x86_code(hz_xz_simple_x86 *simple, uint32_t now_pos,
                       int is_encoder, uint8_t *buffer, size_t size) {
    static const uint32_t mask_to_bit_number[5] = { 0, 1, 2, 2, 3 };
    uint32_t prev_mask = simple->prev_mask;
    uint32_t prev_pos = simple->prev_pos;
    if (size < 5) return 0;
    if (now_pos - prev_pos > 5) prev_pos = now_pos - 5;
    const size_t limit = size - 5;
    size_t buffer_pos = 0;
    while (buffer_pos <= limit) {
        uint8_t b = buffer[buffer_pos];
        if (b != 0xE8 && b != 0xE9) { ++buffer_pos; continue; }
        const uint32_t offset = now_pos + (uint32_t)buffer_pos - prev_pos;
        prev_pos = now_pos + (uint32_t)buffer_pos;
        if (offset > 5) prev_mask = 0;
        else for (uint32_t i = 0; i < offset; ++i) {
            prev_mask &= 0x77;
            prev_mask <<= 1;
        }
        b = buffer[buffer_pos + 4];
        if (Test86MSByte(b) && (prev_mask >> 1) <= 4 && (prev_mask >> 1) != 3) {
            uint32_t src = ((uint32_t)b << 24) | ((uint32_t)buffer[buffer_pos + 3] << 16) |
                           ((uint32_t)buffer[buffer_pos + 2] << 8) | buffer[buffer_pos + 1];
            uint32_t dest;
            for (;;) {
                dest = is_encoder ? src + now_pos + (uint32_t)buffer_pos + 5
                                  : src - now_pos - (uint32_t)buffer_pos - 5;
                if (prev_mask == 0) break;
                const uint32_t i = mask_to_bit_number[prev_mask >> 1];
                b = (uint8_t)(dest >> (24 - i * 8));
                if (!Test86MSByte(b)) break;
                src = dest ^ ((1U << (32 - i * 8)) - 1);
            }
            buffer[buffer_pos + 4] = (uint8_t)(~(((dest >> 24) & 1) - 1));
            buffer[buffer_pos + 3] = (uint8_t)(dest >> 16);
            buffer[buffer_pos + 2] = (uint8_t)(dest >> 8);
            buffer[buffer_pos + 1] = (uint8_t)dest;
            buffer_pos += 5;
            prev_mask = 0;
        } else {
            ++buffer_pos;
            prev_mask |= 1;
            if (Test86MSByte(b)) prev_mask |= 0x10;
        }
    }
    simple->prev_mask = prev_mask;
    simple->prev_pos = prev_pos;
    return buffer_pos;
}

size_t hz_xz_bcj_x86_encode(uint32_t start_offset, uint8_t *buffer, size_t size) {
    hz_xz_simple_x86 simple = { 0, UINT32_MAX - 4 };
    return x86_code(&simple, start_offset, 1, buffer, size);
}

size_t hz_xz_bcj_x86_decode(uint32_t start_offset, uint8_t *buffer, size_t size) {
    hz_xz_simple_x86 simple = { 0, UINT32_MAX - 4 };
    return x86_code(&simple, start_offset, 0, buffer, size);
}
