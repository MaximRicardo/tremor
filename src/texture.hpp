#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class Texture {

    // a list of palette idxs
    std::vector<uint8_t> pixels;
    uint32_t width;
    uint32_t height;

public:
    std::string name;

    Texture(std::span<const uint8_t> pixels, uint32_t width, uint32_t height,
            std::string_view name);

    const std::vector<uint8_t> &get_pixels() const;
    uint32_t get_width() const;
    uint32_t get_height() const;
};
