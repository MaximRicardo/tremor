#pragma once

#include <cstdint>

// needs to be packed to guarantee alignment with uint8_t arrays
class Color {

public:
    uint8_t r = 0, g = 0, b = 0, a = 255;

    Color() = default;

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
} __attribute__((packed));
