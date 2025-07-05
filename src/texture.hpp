#pragma once

#include "color.hpp"
#include <cstdint>
#include <string>
#include <vector>

class Texture {

    std::vector<Color> pixels;
    uint32_t width;
    uint32_t height;

public:
    void load(std::string file_path);

    const std::vector<Color> &get_pixels() const;
    uint32_t get_width() const;
    uint32_t get_height() const;
};
