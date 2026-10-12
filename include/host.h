#pragma once

#include "device.h"
#include "median_filter.h"

// Configure the register map, trigger START, then wait for the completion interrupt.
Status run_median_filter(Device& device, const std::uint8_t* input,
                  std::uint8_t* output, const ImageParams& params, int radius);


Status run_sobel_filter(Device& device, const std::uint8_t* input,
    std::uint8_t* output, const ImageParams& params, double gain);
