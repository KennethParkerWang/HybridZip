#include <stdint.h>

// libFLAC's LPC quantizer references this format constant from format.c.
// HybridZip retains only the residual-analysis closure, so it supplies the
// published FLAC-format value without importing container metadata code.
const uint32_t FLAC__SUBFRAME_LPC_QLP_SHIFT_LEN = 5U;
