#pragma once

#include <cstddef>
#include <cstdint>

// Shared host/model communication API. See ../REGISTER_MAP.md for the contract.
// These are logical offsets; each slot carries a native uintptr_t value.
constexpr unsigned kRegisterSpacing = 0x08;
constexpr std::size_t kRegisterCount = 8;

enum class Register : unsigned {
    InputAddress = 0x00, // Host writes: address of uint8_t input pixels.
    OutputAddress = 0x08, // Host writes: address of uint8_t output pixels.
    ParamsAddress = 0x10, // Host writes: address of ImageParams (median_filter.h).
    Radius = 0x18, // Host writes: median radius, 0 through 8.
    Command = 0x20, // Host writes: Start signals the request event.
    Status = 0x28,  // Model writes; host reads after completion.
    FilterSelect = 0x30, // 0 = median, 1 = Sobel. Other values are invalid.
    EdgeGain = 0x38 // Represents real-valued gain in [0.0, 1.0] using an unsigned integer code.  Byte presentation is real value myltiplyed by 1 << 10 (Q22.10)sents zero gain.
};

enum class Command : std::uintptr_t { Start = 1 };
enum class Status : std::uintptr_t { Idle = 0, Done = 1, InvalidArgument = 2 };
enum class FilterSelect : std::uintptr_t { Median = 0, Sobel = 1 };
