// Extracted from C-Blosc2 blosc/shuffle-generic.c at commit
// b17d0c3dae8d48800726a85455d9f1fdf0578b16. BSD-3-Clause.

#include "shuffle_generic.h"
#include <string.h>

void hz_blosc_shuffle_generic(uint32_t type_size, size_t block_size, const uint8_t *source, uint8_t *destination) {
    const size_t elements = block_size / type_size;
    const size_t remainder = block_size % type_size;
    for (uint32_t byte = 0; byte < type_size; ++byte)
        for (size_t element = 0; element < elements; ++element)
            destination[(size_t)byte * elements + element] = source[element * type_size + byte];
    memcpy(destination + block_size - remainder, source + block_size - remainder, remainder);
}

void hz_blosc_unshuffle_generic(uint32_t type_size, size_t block_size, const uint8_t *source, uint8_t *destination) {
    const size_t elements = block_size / type_size;
    const size_t remainder = block_size % type_size;
    for (size_t element = 0; element < elements; ++element)
        for (uint32_t byte = 0; byte < type_size; ++byte)
            destination[element * type_size + byte] = source[(size_t)byte * elements + element];
    memcpy(destination + block_size - remainder, source + block_size - remainder, remainder);
}
