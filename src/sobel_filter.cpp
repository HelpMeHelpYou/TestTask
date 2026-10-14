#include "sobel_filter.h"
#include "reference_image.h"

#include <math.h>
#include <bit>

uint32_t fixed_sqrt(uint32_t n) {
    if (n == 0) return 0;

    // Use a wider type (64-bit) during iteration to prevent overflow
    int64_t n_scaled = int64_t(n) << SHIFT;
    int64_t x = int64_t(n) << (SHIFT / 2); // Initial guess
    int64_t x_old = 0;

    while (true) {
        x_old = x;
        x = (x + n_scaled / x) / 2;
        if (abs(x - x_old) <= 1) break;
    }

    return static_cast<uint32_t>(x);
}

uint64_t isqrt_bitwise(uint64_t x) {
    uint64_t res = 0;
    uint64_t bit = uint64_t(1) << 62;  // Highest power of 4 <= 2^64

    // Find the highest power of 4 <= x
    while (bit > x) bit >>= 2;

    while (bit != 0) {
        uint64_t trial = res + bit;
        if (x >= trial) {
            x -= trial;
            res = (res >> 1) + bit;
        }
        else {
            res >>= 1;
        }
        bit >>= 2;
    }

    return res;
}

int evalute_sobel_value(int gx, int gy, uint32_t gain_shifted)
{
    return  (fixed_sqrt((gx * gx + gy * gy) * ONE) * gain_shifted / 4 + ONE / 2) / (ONE * ONE);
}

void sobel_filter(const uint8_t* input, uint8_t* output,
    const ImageParams* params, unsigned int edgeGain)
{
    static const int kernel_x[3][3] = {
        {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}
    };
    static const int kernel_y[3][3] = {
        {-1, -2, -1}, {0, 0, 0}, {1, 2, 1}
    };
    const ReferenceImage image = make_reference_image(input, params->width, params->height, params->input_stride, 1);
    int y;
    for (y = 0; y < params->height; ++y) {
        int x;
        for (x = 0; x < params->width; ++x) {
            /* The reflected border makes every 3x3 neighborhood available. */
            const uint8_t* window = image.pixels + (size_t)y * image.stride + x;
            int gx = 0;
            int gy = 0;
            int ky;
            int value;
         
            for (ky = 0; ky < 3; ++ky) {
                const uint8_t* row = window + (size_t)ky * image.stride;
                int kx;
                for (kx = 0; kx < 3; ++kx) {
                    const int pixel = row[kx];
                    gx += kernel_x[ky][kx] * pixel;
                    gy += kernel_y[ky][kx] * pixel;
                }
            }
            value = evalute_sobel_value(gx, gy, edgeGain);

            int value2;
            value2 = (fixed_sqrt((gx * gx + gy * gy) * ONE) / 4 + ONE / 2) / ONE;
            double value1;
            value1 =   sqrt((double)gx * gx + (double)gy * gy) / 4.0;
            double value3 = sqrt((double)gx * gx + (double)gy * gy) / 4.0 * 0.7;

            if (value2 != static_cast<int32_t>(value1+0.5))
            {
                printf("");
            }
            if (value != static_cast<int32_t>(value3 + 0.5))
            {
                printf("");
            }
        

            output[(size_t)y * params->width + x] = value > 255 ? 255 : value;
        }
    }


}