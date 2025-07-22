#include "wad2.hpp"
#include "../index.hpp"
#include "../utils/bin_data.hpp"
#include "wad.hpp"
#include <array>
#include <cassert>
#include <cstdint>
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

int32_t WAD::WAD2MipHeader::mipmap_lvl_pos(unsigned lvl) const
{
    switch (lvl) {

    case 0:
        return this->scale_1_pos;

    case 1:
        return this->scale_2_pos;

    case 2:
        return this->scale_4_pos;

    case 3:
        return this->scale_8_pos;

    default:
        assert(false);
    };
}

std::vector<uint8_t>
WAD::WAD2MipHeader::get_pixels(std::span<const uint8_t> data,
                               unsigned mipmap_lvl) const
{
    std::vector<uint8_t> pixels;
    pixels.reserve(this->width * this->height);

    size_t base_offset = this->mipmap_lvl_pos(mipmap_lvl) + this->own_offset;
    // base_offset += (this->n_mipmap_lvls - mipmap_lvl - 1) * 4;

    int32_t mipmap_width = this->width >> mipmap_lvl;
    int32_t mipmap_height = this->height >> mipmap_lvl;

    for (int32_t y = 0; y < mipmap_width; ++y) {
        for (int32_t x = 0; x < mipmap_height; ++x) {
            size_t offset =
                Index::conv_2d_to_1d(Vec2i(x, y), mipmap_width) + base_offset;
            uint8_t idx = BinData::read_num<uint8_t>(data, offset);
            pixels.push_back(idx);
        }
    }

    return pixels;
}

Texture WAD::WAD2MipHeader::to_texture(std::span<const uint8_t> data) const
{
    std::array<MipMapLevel, n_mipmap_lvls> mipmaps;
    for (size_t i = 0; i < this->n_mipmap_lvls; ++i) {
        mipmaps[i] = MipMapLevel(this->get_pixels(data, i));
    }

    return Texture(mipmaps, this->width, this->height, this->name);
}

WAD::WAD2Entry::WAD2Entry(std::span<const uint8_t> data, size_t offset)
{
    this->offset = BinData::read_num<int32_t>(data, offset + offset_offset);
    this->d_size = BinData::read_num<int32_t>(data, offset + d_size_offset);
    this->size = BinData::read_num<int32_t>(data, offset + size_offset);
    this->type = BinData::read_num<int8_t>(data, offset + type_offset);
    this->is_comprsd =
        BinData::read_num<int8_t>(data, offset + is_comprsd_offset);
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
    if (mip.name != this->name)
        throw std::runtime_error("error: mipmap texture name '" + mip.name +
                                 "' does not match entry texture name '" +
                                 this->name + "'");

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

    WAD2Dir dir(header, data);
    return dir.get_textures(data);
}
