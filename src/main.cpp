#include "host.h"
#include "y4m.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc > 4 || (argc > 1 && std::string(argv[1]) == "--help")) {
        std::cout << "Usage: median_demo [input.y4m] [radius] [output-prefix]\n"
                  << "Defaults: data/lena_mono.y4m 1\n"
                  << "Optional prefix saves <prefix>_device.y4m and <prefix>_reference.y4m\n";
        return argc > 4 ? 2 : 0;
    }
    const std::string input_path = argc > 1 ? argv[1] : "data/lena_mono.y4m";
    int radius = 1;
    if (argc > 2) {
        std::istringstream value(argv[2]);
        if (!(value >> radius) || value.peek() != std::char_traits<char>::eof() ||
            radius < 0 || radius > MEDIAN_MAX_RADIUS) {
            std::cerr << "Radius must be an integer from 0 to " << MEDIAN_MAX_RADIUS << '\n';
            return 2;
        }
    }

    MonoImage input;
    std::string error;
    if (!read_y4m(input_path, input, error)) {
        std::cerr << error << '\n';
        return 2;
    }
    std::vector<std::uint8_t> actual(input.pixels.size());
    std::vector<std::uint8_t> expected(input.pixels.size());
    const ImageParams params{input.width, input.height, input.width, input.width};
    Device device;

    std::cout << "Input: " << input_path << " (" << input.width << 'x' << input.height
              << ", 8-bit monochrome), radius=" << radius << '\n';
    if (run_median_filter(device, input.pixels.data(), actual.data(), params, radius) != Status::Done) {
        std::cerr << "The model rejected the job\n";
        return 2;
    }

    // The device has finished writing actual. Run the reference and compare memory.
    median_reference(input.pixels.data(), expected.data(), &params, radius);
    std::size_t mismatches = 0;
    for (std::size_t i = 0; i < actual.size(); ++i) {
        if (actual[i] != expected[i]) {
            if (mismatches == 0) {
                std::cout << "First mismatch at (" << i % input.width << ", " << i / input.width
                          << "): expected=" << static_cast<int>(expected[i])
                          << " actual=" << static_cast<int>(actual[i]) << '\n';
            }
            ++mismatches;
        }
    }
    std::cout << mismatches << " mismatching pixel(s) of " << actual.size() << '\n';

    // Save both buffers even on a mismatch, so the defect can be inspected visually.
    if (argc > 3) {
        const std::string prefix = argv[3];
        const std::string device_path = prefix + "_device.y4m";
        const std::string reference_path = prefix + "_reference.y4m";
        if (!write_y4m(device_path, actual.data(), params.width, params.height,
                       params.output_stride, error) ||
            !write_y4m(reference_path, expected.data(), params.width, params.height,
                       params.output_stride, error)) {
            std::cerr << error << '\n';
            return 2;
        }
        std::cout << "Saved " << device_path << " and " << reference_path << '\n';
    }
    return mismatches == 0 ? 0 : 1;
}
