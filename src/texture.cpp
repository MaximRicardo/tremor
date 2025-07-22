#include "texture.hpp"
#include "stb_image.h"
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <vector>

Texture::Texture(std::span<const uint8_t> pixels, uint32_t width,
                 uint32_t height, std::string_view name)
    : width(width), height(height), name(std::string(name))
{
    this->pixels.assign(pixels.begin(), pixels.end());
}

const std::vector<uint8_t> &Texture::get_pixels() const
{
    return this->pixels;
}

uint32_t Texture::get_width() const
{
    return this->width;
}

uint32_t Texture::get_height() const
{
    return this->height;
}
