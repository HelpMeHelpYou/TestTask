#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include "base_filter.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Buffers do not overlap. The caller owns the buffers and descriptor. */
void median_reference(const uint8_t* input, uint8_t* output,
                      const ImageParams* params, int radius);
void median_histogram(const uint8_t* input, uint8_t* output,
                      const ImageParams* params, int radius);
int reflect101(int coordinate, int length);

#ifdef __cplusplus
}
#endif

#endif
