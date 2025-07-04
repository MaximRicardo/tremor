#include "time.hpp"
#include <chrono>
#include <cstdint>

std::chrono::steady_clock::time_point start;

void Time::init()
{
    start = std::chrono::steady_clock::now();
}

uint32_t Time::get_ticks_ms()
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
        .count();
}
