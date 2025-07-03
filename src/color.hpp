#pragma once

#include <cstdint>

class Color {

public:
    std::uint8_t r, g, b, a;

    Color(std::uint8_t r = 0, std::uint8_t g = 0, std::uint8_t b = 0,
          std::uint8_t a = 255);
} __attribute__((packed));
