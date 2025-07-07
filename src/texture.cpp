#include "texture.hpp"
#include "stb_image.h"
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

void Texture::load(std::string file_path)
{
    std::ifstream file(file_path, std::ios::binary);
    if (file.fail()) {
        throw std::runtime_error("can't open image file " + file_path + ": " +
                                 std::strerror(errno));
    }

    int width, height, channels;
    stbi_uc *data = stbi_load(file_path.c_str(), &width, &height, &channels, 3);

    if (channels != 3) {
        throw std::runtime_error("image " + file_path + "has " +
                                 std::to_string(channels) +
                                 "instead of the required 3\n");
    }

    this->width = width;
    this->height = height;
    this->pixels.clear();
    for (size_t i = 0; i < this->width * this->height * 3; i += 3) {
        this->pixels.emplace_back(data[i + 0], data[i + 1], data[i + 2]);
    }

    free(data);
}

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
