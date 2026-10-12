#include "sobel_reference.h"
#include "y4m.h"
#include "host.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc > 4 || (argc > 1 && std::string(argv[1]) == "--help")) {
        std::cout << "Usage: sobel_reference_demo [input.y4m] [gain] [output.y4m]\n"
                  << "Defaults: data/lena_mono.y4m 1 sobel\n";
        return argc > 4 ? 2 : 0;
    }
    const std::string input_path = argc > 1 ? argv[1] : "data/lena_mono.y4m";
    const std::string output_path = argc > 3 ? argv[3] : "sobel";
    double gain = 1.0;
    if (argc > 2) {
        std::istringstream value(argv[2]);
        if (!(value >> gain) || value.peek() != std::char_traits<char>::eof() ||
            !std::isfinite(gain) || gain < 0.0 || gain > 1.0) {
            std::cerr << "Gain must be a finite number from 0 to 1\n";
            return 2;
        }
    }

    MonoImage input;
    std::string error;
    if (!read_y4m(input_path, input, error)) {
        std::cerr << error << '\n';
        return 2;
    }


    std::vector<double> reference(input.pixels.size());
    std::vector<std::uint8_t> actual(reference.size());

    Device device;
    const ImageParams params{ input.width, input.height, input.width, input.width };

    if (run_sobel_filter(device, input.pixels.data(), actual.data(), params, gain) != Status::Done) {
        std::cerr << "The model rejected the job\n";
        return 2;
    }

    sobel_reference(input.pixels.data(), input.width, input.height, input.width,
                    gain, reference.data());

    // Quantize only the preview. The reference buffer retains fractional values.
    std::vector<std::uint8_t> preview(reference.size());
    for (std::size_t i = 0; i < reference.size(); ++i) {
        preview[i] = static_cast<std::uint8_t>(reference[i] + 0.5);
    }

    std::array<std::size_t ,256> histogram{};
    std::size_t mismatches = 0;
    for (std::size_t i = 0; i < actual.size(); ++i) {
        size_t diff = abs(actual[i] - preview[i]);
        histogram[diff]++;

        if (actual[i] != preview[i]) {
            if (mismatches == 0) {
                std::cout << "First mismatch at (" << i % input.width << ", " << i / input.width
                    << "): expected=" << static_cast<int>(preview[i])
                    << " actual=" << static_cast<int>(actual[i]) << '\n';
            }
            ++mismatches;
        }
    }

    bool error_is_too_big = false;
    std::cout << "Error histogram is:" << std::endl;
    for (size_t i=0; i< histogram.size(); i++)
    {
        if (histogram[i])
        {
            std::cout << i << " : " << histogram[i] << std::endl;
            if (i > 1)
            {
                error_is_too_big = true;
            }
        }
    }
    if (error_is_too_big)
        std::cout << "error is too much, unacceptable!" << std::endl;

    if (!write_y4m(output_path+ "_reference.y4m", preview.data(), input.width, input.height,
                   input.width, error)) {
        std::cerr << error << '\n';
        return 2;
    }

    if (!write_y4m(output_path + "_device.y4m", preview.data(), input.width, input.height,
        input.width, error)) {
        std::cerr << error << '\n';
        return 2;
    }

    std::cout << "Floating-point reference: " << input_path << " ("
              << input.width << 'x' << input.height << "), gain=" << gain
              << "\nSaved 8-bit preview: " << output_path + "_reference.y4m" << '\n';
    return 0;
}
