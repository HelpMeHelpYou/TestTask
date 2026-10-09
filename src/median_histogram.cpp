#include "median_filter.h"

#include <array>
#include <cstddef>
#include <cstdint>

int reflect101(int coordinate, int length) {
    if (length == 1) return 0;
    if (coordinate < 0) coordinate = -coordinate;
    if (coordinate >= length) coordinate = 2 * (length - 1) - coordinate;
    return coordinate < 0 ? reflect101(coordinate,length) : coordinate;
}


namespace {
std::uint8_t select_median(const std::array<int, 256>& histogram, int samples) {
    int count = 0;
    for (int value = 0; value < 256; ++value) {
        count += histogram[value];
        if (count > samples / 2) return static_cast<std::uint8_t>(value);
    }
    return 255;
}
} // namespace

void median_histogram(const std::uint8_t* input, std::uint8_t* output,
                      const ImageParams* params, int radius) {
    const int diameter = 2 * radius + 1;
    const int samples = diameter * diameter;

    for (int y = 0; y < params->height; ++y) {
        std::array<int, 256> histogram{};

        // Initialize the window centered at x=0 for this row.
        for (int dy = -radius; dy <= radius; ++dy) {
            const int row = reflect101(y + dy, params->height);
            const auto base = static_cast<std::size_t>(row) * params->input_stride;
            for (int dx = -radius; dx <= radius; ++dx) {
                ++histogram[input[base + reflect101(dx, params->width)]];
            }
        }

        for (int x = 0; x < params->width; ++x) {
            output[static_cast<std::size_t>(y) * params->output_stride + x] =
                select_median(histogram, samples);

            if (x + 1 == params->width) break;

            // Slide the window one pixel right: remove a column and add a column.
            const int leaving = reflect101(x - radius, params->width);
            const int entering = reflect101(x + radius + 1, params->width);
            for (int dy = -radius; dy <= radius; ++dy) {
                const int row = reflect101(y + dy, params->height);
                const auto base = static_cast<std::size_t>(row) * params->input_stride;
                --histogram[input[base + leaving]];
                ++histogram[input[base + entering]];
            }
        }
    }
}
