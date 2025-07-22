#include "wad2.hpp"
#include "../utils/bin_data.hpp"
#include "wad.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// offset is the offset of the name in the file
std::string get_name(std::span<const uint8_t> data, size_t offset)
{
    std::string name;
    for (size_t i = offset; data[i] != '\0'; ++i) {
        name.push_back(data[i]);
    }

    return name;
}

} // namespace

WAD::WAD2MipHeader::WAD2MipHeader(std::span<const uint8_t> data, size_t offset)
    : own_offset(offset)
{
    this->name = get_name(data, this->own_offset + name_offset);
    this->width =
        BinData::read_num<int32_t>(data, this->own_offset + width_offset);
    this->height =
        BinData::read_num<int32_t>(data, this->own_offset + height_offset);
    this->scale_1_pos =
        BinData::read_num<int32_t>(data, this->own_offset + scale_1_pos_offset);
    this->scale_2_pos =
        BinData::read_num<int32_t>(data, this->own_offset + scale_2_pos_offset);
    this->scale_4_pos =
        BinData::read_num<int32_t>(data, this->own_offset + scale_4_pos_offset);
    this->scale_8_pos =
        BinData::read_num<int32_t>(data, this->own_offset + scale_8_pos_offset);
}

std::vector<uint8_t>
WAD::WAD2MipHeader::get_pixels(std::span<const uint8_t> data,
                               unsigned mipmap_lvl) const
{
    // only the first mipmap is supported rn
    assert(mipmap_lvl == 0);

    std::vector<uint8_t> pixels;
    pixels.reserve(this->width * this->height);

    size_t base_offset = this->scale_1_pos + 4 + this->own_offset;

    for (int32_t y = 0; y < this->height; ++y) {
        for (int32_t x = 0; x < this->width; ++x) {
            size_t offset = (this->width * y + x) + base_offset;
            uint8_t idx = BinData::read_num<uint8_t>(data, offset);
            pixels.push_back(idx);
        }
    }

    return pixels;
}

Texture WAD::WAD2MipHeader::to_texture(std::span<const uint8_t> data) const
{
    return Texture(this->get_pixels(data, 0), this->width, this->height,
                   this->name);
}

WAD::WAD2Entry::WAD2Entry(std::span<const uint8_t> data, size_t offset)
{
    this->offset = BinData::read_num<int32_t>(data, offset + offset_offset);
    this->d_size = BinData::read_num<int32_t>(data, offset + d_size_offset);
    this->size = BinData::read_num<int32_t>(data, offset + size_offset);
    this->type = BinData::read_num<int8_t>(data, offset + type_offset);
    this->is_comprsd =
        BinData::read_num<int8_t>(data, offset + is_comprsd_offset);
    // unused int16_t here
    this->name = get_name(data, offset + this->name_offset);

    if (this->is_comprsd)
        throw std::runtime_error(
            "error: compressed WAD2 entries are not supported.");
}

WAD::WAD2MipHeader
WAD::WAD2Entry::read_mip_tex(std::span<const uint8_t> data) const
{
    assert(this->type == this->mip_tex_type);

    WAD2MipHeader mip(data, this->offset);

    std::cout << "name = " << mip.name << "\n";
    std::cout << "width = " << mip.width << "\n";
    std::cout << "height = " << mip.height << "\n";
    std::cout << "scale_1_pos = " << mip.scale_1_pos << "\n";
    std::cout << "scale_2_pos = " << mip.scale_2_pos << "\n";
    std::cout << "scale_4_pos = " << mip.scale_4_pos << "\n";
    std::cout << "scale_8_pos = " << mip.scale_8_pos << "\n";

    return mip;
}

WAD::WAD2Dir::WAD2Dir(const WAD2Header &header, std::span<const uint8_t> data)
{
    this->entries.reserve(header.n_entries);

    size_t offset = header.dir_offset;
    for (int32_t i = 0; i < header.n_entries; ++i) {
        this->entries.emplace_back(data, offset);
        offset += WAD::WAD2Entry::entry_offset_inc;
    }
}

std::vector<Texture>
WAD::WAD2Dir::get_textures(std::span<const uint8_t> data) const
{
    std::vector<Texture> texs;

    for (const auto &entry : this->entries) {
        if (entry.type != WAD2Entry::mip_tex_type)
            continue;

        texs.push_back(entry.read_mip_tex(data).to_texture(data));
    }

    return texs;
}

WAD::WAD2Header::WAD2Header(std::span<const uint8_t> data, size_t offset)
    : Header(data, offset)
{}

std::vector<Texture> WAD::load_wad2(std::span<const uint8_t> data)
{
    WAD2Header header(data, 0);
    assert(header.format == wad2_format_name);

    std::cout << "n entries = " << header.n_entries << "\n";
    std::cout << "dir offset = " << header.dir_offset << "\n";

    WAD2Dir dir(header, data);
    return dir.get_textures(data);
}
