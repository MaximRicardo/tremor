#pragma once

#include <cstdint>

// needs to be packed to guarantee alignment with uint8_t arrays
class Color {

public:
    uint8_t r, g, b, a;

    Color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255);
} __attribute__((packed));
