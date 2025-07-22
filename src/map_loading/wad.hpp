#pragma once

#include "../texture.hpp"
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace WAD {

constexpr size_t format_len = 4;

class Header {

public:
    std::string format;
    int32_t n_entries;
    int32_t dir_offset;

    Header(std::span<const uint8_t> data, size_t offset);
};

std::vector<Texture> load_file(const std::filesystem::path &path);

}; // namespace WAD
