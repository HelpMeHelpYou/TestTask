#ifndef SOBEL_FILTER_H
#define SOBEL_FILTER_H

#include "base_filter.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* Buffers do not overlap. The caller owns the buffers and descriptor. */
    void sobel_filter(const uint8_t* input, uint8_t* output,
        const ImageParams* params, unsigned int edgeGain);

 // avoid buffer overflow in worst case
//  checked with test
    constexpr int SHIFT = 10;
    constexpr int64_t ONE = (int64_t(1) << SHIFT);


    int evalute_sobel_value(int gx, int gy, uint32_t gain_shifted);
#ifdef __cplusplus
}
#endif

#endif //SOBEL_FILTER_H

