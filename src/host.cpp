#include "host.h"
#include "sobel_filter.h"

Status run_median_filter(Device& device, const std::uint8_t* input,
                  std::uint8_t* output, const ImageParams& params, int radius) {
    device.write_register(Register::InputAddress, reinterpret_cast<std::uintptr_t>(input));
    device.write_register(Register::OutputAddress, reinterpret_cast<std::uintptr_t>(output));
    device.write_register(Register::ParamsAddress, reinterpret_cast<std::uintptr_t>(&params));
    device.write_register(Register::Radius, static_cast<std::uintptr_t>(radius));
    device.write_register(Register::FilterSelect, static_cast<std::uintptr_t>(FilterSelect::Median));
    device.write_register(Register::Command, static_cast<std::uintptr_t>(Command::Start));
    device.wait_interrupt();
    return static_cast<Status>(device.read_register(Register::Status));
}


Status run_sobel_filter(Device& device, const std::uint8_t* input,
    std::uint8_t* output, const ImageParams& params,double gain)
{
    device.write_register(Register::InputAddress, reinterpret_cast<std::uintptr_t>(input));
    device.write_register(Register::OutputAddress, reinterpret_cast<std::uintptr_t>(output));
    device.write_register(Register::ParamsAddress, reinterpret_cast<std::uintptr_t>(&params));
    uint32_t byteGain = static_cast<int32_t> (ONE * gain);
    device.write_register(Register::EdgeGain, static_cast<std::uintptr_t>(byteGain));
    device.write_register(Register::FilterSelect, static_cast<std::uintptr_t>(FilterSelect::Sobel));
    device.write_register(Register::Command, static_cast<std::uintptr_t>(Command::Start));
    device.wait_interrupt();
    return static_cast<Status>(device.read_register(Register::Status));

}

