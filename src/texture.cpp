#include "texture.hpp"
#include "texture_raylib.hpp"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

void Texture::load(std::string file_path)
{
    auto raylib_ret = TextureRaylib::load_image(file_path);

    this->width = raylib_ret.width;
    this->height = raylib_ret.height;

    this->pixels.clear();
    printf("n raylib pixels = %zu\n", raylib_ret.data.size());
    for (size_t i = 0; i < raylib_ret.width * raylib_ret.height * 3; i += 3) {
        Color color;
        color.r = raylib_ret.data[i];
        color.g = raylib_ret.data[i + 1];
        color.b = raylib_ret.data[i + 2];
        this->pixels.push_back(color);
    }
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
