#pragma once

#include "color.hpp"
#include <cstdint>
#include <filesystem>
#include <vector>

class Texture {

    std::vector<Color> pixels;
    uint32_t width;
    uint32_t height;

    void load(const std::filesystem::path &path);

public:
    Texture(const std::filesystem::path &path);

    const std::vector<Color> &get_pixels() const;
    uint32_t get_width() const;
    uint32_t get_height() const;
};
