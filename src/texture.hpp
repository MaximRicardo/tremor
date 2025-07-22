#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class Texture;

class MipMapLevel {

    std::vector<uint8_t> pixels;

public:
    MipMapLevel() = default;

    MipMapLevel(std::span<const uint8_t> pixels);

    const std::vector<uint8_t> &get_pixels() const;
};

class Texture {

public:
    static constexpr size_t n_mipmap_lvls = 4;

private:
    std::vector<MipMapLevel> mipmaps;
    uint32_t width;
    uint32_t height;

public:
    std::string name;

    Texture(std::span<const MipMapLevel, n_mipmap_lvls> mipmaps, uint32_t width,
            uint32_t height, std::string_view name);

    const std::vector<uint8_t> &get_pixels(size_t mipmap_lvl) const;
    uint32_t get_width(size_t mipmap_lvl) const;
    uint32_t get_height(size_t mipmap_lvl) const;
    uint32_t n_pixels(size_t mipmap_lvl) const;
};
