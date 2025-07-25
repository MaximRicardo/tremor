#include "texture.hpp"
#include "utils/string.hpp"
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <vector>

MipMapLevel::MipMapLevel(std::span<const uint8_t> pixels)
{
    this->pixels.assign(pixels.begin(), pixels.end());
}

const std::vector<uint8_t> &MipMapLevel::get_pixels() const
{
    return this->pixels;
}

Texture::Texture(std::span<const MipMapLevel, n_mipmap_lvls> mipmaps,
                 int32_t width, int32_t height, std::string_view name)
    : width(width), height(height), name(String::str_tolower(std::string(name)))
{
    this->mipmaps.assign(mipmaps.begin(), mipmaps.end());
}

const std::vector<uint8_t> &Texture::get_pixels(size_t mipmap_lvl) const
{
    return this->mipmaps[mipmap_lvl].get_pixels();
}

int32_t Texture::get_width(size_t mipmap_lvl) const
{
    return this->width >> mipmap_lvl;
}

int32_t Texture::get_height(size_t mipmap_lvl) const
{
    return this->height >> mipmap_lvl;
}

uint32_t Texture::n_pixels(size_t mipmap_lvl) const
{
    return this->get_width(mipmap_lvl) * this->get_height(mipmap_lvl);
}
