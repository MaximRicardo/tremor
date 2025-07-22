#pragma once

#include "color.hpp"
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class Texture {

    std::vector<Color> pixels;
    uint32_t width;
    uint32_t height;

    // void load(const std::filesystem::path &path);

public:
    std::string name;

    Texture(std::span<const Color> pixels, uint32_t width, uint32_t height,
            std::string_view name);

    const std::vector<Color> &get_pixels() const;
    uint32_t get_width() const;
    uint32_t get_height() const;
};
