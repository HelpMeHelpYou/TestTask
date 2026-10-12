#include "device.h"
#include "median_filter.h"
#include "sobel_filter.h"

Device::Device() : worker_(&Device::run, this) {}

Device::~Device() {
    stopping_.store(true);
    request_.signal();
    worker_.join();
}

void Device::write_register(Register reg, std::uintptr_t value) {
    registers_[static_cast<unsigned>(reg) / kRegisterSpacing] = value;
    if (reg == Register::Command) {
        request_.signal();
    }
}

std::uintptr_t Device::read_register(Register reg) const {
    return registers_[static_cast<unsigned>(reg) / kRegisterSpacing];
}

void Device::wait_interrupt() {
    completion_.wait();
}

void Device::run() {
    for (;;) {
        request_.wait();
        if (stopping_.load()) return;

        const auto* input = reinterpret_cast<const std::uint8_t*>(
            read_register(Register::InputAddress));
        auto* output = reinterpret_cast<std::uint8_t*>(
            read_register(Register::OutputAddress));
        const auto* descriptor = reinterpret_cast<const ImageParams*>(
            read_register(Register::ParamsAddress));

        const auto filetSelect = static_cast<FilterSelect>(
            read_register(Register::FilterSelect));

        Status status = Status::InvalidArgument;

        // Valid allocated buffers and their lifetime are the host's responsibility.
        if (read_register(Register::Command) == static_cast<std::uintptr_t>(Command::Start) &&
            input && output && descriptor && input != output)

        {
            const ImageParams params = *descriptor;
            if (params.width > 0 && params.height > 0 &&
                params.width <= MEDIAN_MAX_DIMENSION && params.height <= MEDIAN_MAX_DIMENSION &&
                params.input_stride >= params.width && params.output_stride >= params.width)
                switch (filetSelect)
                {

                case FilterSelect::Median:
                {
                    const auto radius = read_register(Register::Radius);

                    if (radius <= MEDIAN_MAX_RADIUS)
                    {
                        median_histogram(input, output, &params, static_cast<int>(radius));
                        status = Status::Done;
                    }
                    break;
                }
                case  FilterSelect::Sobel:
                {
                    const auto edgeGain = read_register(Register::EdgeGain);
                    if (edgeGain <= ONE)
                    {
                        sobel_filter(input, output, &params, static_cast<unsigned int>(edgeGain));
                        status = Status::Done;
                    }
                    break;
                }
                default:
                    break;
                }
        }

        // Completion also publishes output writes to the waiting host.
        registers_[static_cast<unsigned>(Register::Status) / kRegisterSpacing] =
            static_cast<std::uintptr_t>(status);
        completion_.signal();
    }
}
