#pragma once

#include "event.h"
#include "register_map.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <thread>

class Device {
public:
    Device();
    ~Device();
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // One host, one outstanding job. Configure only while idle/after waiting.
    void write_register(Register reg, std::uintptr_t value);
    std::uintptr_t read_register(Register reg) const;
    void wait_interrupt();

private:
    void run();

    std::array<std::uintptr_t, kRegisterCount> registers_{};
    Event request_;
    Event completion_;
    std::atomic<bool> stopping_{false};
    std::thread worker_; // Constructed last: the worker uses all fields above.
};
