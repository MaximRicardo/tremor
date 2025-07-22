#include "wad.hpp"
#include "../texture.hpp"
#include "../utils/bin_data.hpp"
#include "wad2.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

std::vector<Texture> read_file(std::ifstream &file)
{
    std::vector<uint8_t> data(std::istreambuf_iterator<char>(file), {});

    WAD::Header header(data, 0);

    if (header.format == WAD::wad2_format_name)
        return WAD::load_wad2(data);
    else
        throw std::runtime_error("error: unsupported WAD format '" +
                                 header.format + "'\n");
}

}; // namespace

WAD::Header::Header(std::span<const uint8_t> data, size_t offset)
{
    this->format.assign(data.begin() + offset,
                        data.begin() + offset + format_len);
    this->n_entries = BinData::read_num<int32_t>(data, offset + format_len);
    this->dir_offset =
        BinData::read_num<int32_t>(data, offset + format_len + 4);
}

std::vector<Texture> WAD::load_file(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);
    if (file.fail()) {
        throw std::runtime_error("can't open WAD file " + path.string() + ": " +
                                 std::strerror(errno));
    }

    return read_file(file);
}
