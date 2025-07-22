#include "texture.hpp"
#include "stb_image.h"
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <vector>

Texture::Texture(std::span<const Color> pixels, uint32_t width, uint32_t height,
                 std::string_view name)
    : width(width), height(height), name(std::string(name))
{
    this->pixels.assign(pixels.begin(), pixels.end());
}

#ifdef m_COMMENT
void Texture::load(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);
    if (file.fail()) {
        throw std::runtime_error("can't open image file " + path.string() +
                                 ": " + std::strerror(errno));
    }

    int width, height, channels;
    stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 3);

    if (channels < 3) {
        throw std::runtime_error("the image " + path.string() + " has " +
                                 std::to_string(channels) +
                                 ", which is not supported\n");
    }

    this->width = width;
    this->height = height;
    this->pixels.clear();
    for (size_t i = 0; i < this->width * this->height * channels;
         i += channels) {
        this->pixels.emplace_back(data[i + 0], data[i + 1], data[i + 2]);
    }

    free(data);
}
#endif

const std::vector<Color> &Texture::get_pixels() const
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
