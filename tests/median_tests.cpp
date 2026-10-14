#include "device.h"
#include "y4m.h"
#include "median_filter.h"
#include "host.h"
#include "sobel_filter.h"

#include <exception>
#include <iostream>

#define EXPECT_EQ(x,y) if (x!=y) {std::cerr << #x << " not equal " << #y <<std::endl;  return false; }

namespace {
    bool test_device_lifecycle() {
        Device device;
        return true;
    }

    bool test_divice_small_image()
    {
        Device device;

        int radius = 3;
        int sz = 2;
        MonoImage input;
        input.height = sz;
        input.width = sz;
        input.pixels.resize(16);

        for (size_t i = 0; i < input.pixels.size(); i++)
        {
            input.pixels[i] = static_cast<uint8_t>(i % 256);
        }

        std::vector<std::uint8_t> actual(input.pixels.size());
        std::vector<std::uint8_t> expected(input.pixels.size());
        const ImageParams params{ input.width, input.height, input.width, input.width };

        if (run_median_filter(device, input.pixels.data(), actual.data(), params, radius) != Status::Done) {
            std::cerr << "The model rejected the job\n";
            return false;
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
        if (mismatches)
            return false;

        return true;
    }

    bool test_reflect_101()
    {
        //for 4
        // -1 0 1 2 3 4 5 6 7
        //  1 0 1 2 3 2 1 0 1

        EXPECT_EQ(2, reflect101(4, 4))
            EXPECT_EQ(1, reflect101(7, 4))
            EXPECT_EQ(1, reflect101(-7, 4))

            // -1 0 1 2 3 4
            //  1 0 1 0 1 0
            EXPECT_EQ(0, reflect101(2, 2))
            EXPECT_EQ(0, reflect101(4, 2))

            return true;
    }

    bool test_evalute_sobel_value()
    {
        constexpr int maximum = 4 * 255;
        for (double gain = 0; gain < 1.01; gain = gain + 0.1)
            for (int x = -maximum; x < maximum + 1; x=x+8)
                for (int y = -maximum; y < maximum + 1; y=y+8)
                {
                    auto actual = evalute_sobel_value(x, y, static_cast<uint32_t>(gain * ONE));
                    actual = actual > 255 ? 255 : actual;
                    auto value = (gain * sqrt((double)x * x + (double)y * y) / 4.0) + 0.5;
                    auto reference = static_cast<uint8_t>(value > 255.0 ? 255.0 : value);
                    if (abs(actual - reference) > 1)
                        return false;
                }
        return true;
    }

    struct TestCase {
        const char* name;
        bool (*run)();
    };
} // namespace

int main() {
    // Add test functions above and register them here. Return false on failure.
    const TestCase tests[] = {
        {"reflect_101", test_reflect_101},
        {"device_lifecycle", test_device_lifecycle},
        {"device_minimal_image", test_divice_small_image},
        {"evalute_sobel",test_evalute_sobel_value},

    };

    int total = 0;
    int failures = 0;
    for (const auto& test : tests) {
        ++total;
        std::cout << "[ RUN  ] " << test.name << std::endl;
        bool passed = false;
        try {
            passed = test.run();
        }
        catch (const std::exception& error) {
            std::cerr << test.name << ": " << error.what() << '\n';
        }
        catch (...) {
            std::cerr << test.name << ": unknown exception\n";
        }
        std::cout << (passed ? "[ PASS ] " : "[ FAIL ] ") << test.name << '\n';
        if (!passed) ++failures;
    }

    std::cout << total - failures << '/' << total << " tests passed\n";
    return failures == 0 ? 0 : 1;
}
