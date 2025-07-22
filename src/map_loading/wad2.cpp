#include "wad2.hpp"
#include "../palette.hpp"
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
{
    this->name = get_name(data, offset + name_offset);
    this->width = BinData::read_num<int32_t>(data, offset + width_offset);
    this->height = BinData::read_num<int32_t>(data, offset + height_offset);
    this->scale_1_pos =
        BinData::read_num<int32_t>(data, offset + scale_1_pos_offset);
    this->scale_2_pos =
        BinData::read_num<int32_t>(data, offset + scale_2_pos_offset);
    this->scale_4_pos =
        BinData::read_num<int32_t>(data, offset + scale_4_pos_offset);
    this->scale_8_pos =
        BinData::read_num<int32_t>(data, offset + scale_8_pos_offset);
}

std::vector<Color>
WAD::WAD2MipHeader::get_pixels(std::span<const uint8_t> data,
                               unsigned mipmap_lvl,
                               std::span<const Color> palette) const
{
    // only the first mipmap is supported rn
    assert(mipmap_lvl == 0);

    std::vector<Color> pixels;
    pixels.reserve(this->width * this->height);

    size_t base_offset = this->scale_1_pos + 16;
    for (int32_t y = 0; y < this->height; ++y) {
        for (int32_t x = 0; x < this->width; ++x) {
            size_t offset = (this->width * y + x) + base_offset;
            uint8_t idx = BinData::read_num<uint8_t>(data, offset);
            pixels.push_back(palette[idx]);
        }
    }

    return pixels;
}

Texture WAD::WAD2MipHeader::to_texture(std::span<const uint8_t> data,
                                       std::span<const Color> palette) const
{
    return Texture(this->get_pixels(data, 0, palette), this->width,
                   this->height);
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

std::vector<Color>
WAD::WAD2Entry::read_palette(std::span<const uint8_t> data) const
{
    assert(this->type == this->color_palette_type);

    std::vector<Color> palette;

    // entries in the color palette are stored as RGB8
    size_t color_size = sizeof(uint8_t) * 3;
    for (size_t offset = this->offset;
         offset < static_cast<size_t>(this->d_size); offset += color_size) {
        int8_t r = BinData::read_num<int8_t>(data, offset);
        int8_t g = BinData::read_num<int8_t>(data, offset + 1);
        int8_t b = BinData::read_num<int8_t>(data, offset + 2);
        palette.emplace_back(r, g, b);
    }

    return palette;
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

    this->get_palette(data);
}

void WAD::WAD2Dir::set_default_palette()
{
    this->palette = Palette::get_default();
}

void WAD::WAD2Dir::get_palette(std::span<const uint8_t> data)
{
    for (const auto &entry : this->entries) {
        std::cout << "entry type = " << static_cast<char>(entry.type) << "\n";
        if (entry.type != WAD2Entry::color_palette_type)
            continue;

        auto entry_p = entry.read_palette(data);

        // i dunno if ur supposed to just append multiple palettes, but i'ma
        // do that until i find out i'm wrong
        this->palette.insert(this->palette.end(), entry_p.begin(),
                             entry_p.end());
    }

    if (this->palette.empty())
        this->set_default_palette();
}

std::vector<Texture>
WAD::WAD2Dir::get_textures(std::span<const uint8_t> data) const
{
    std::vector<Texture> texs;

    for (const auto &entry : this->entries) {
        if (entry.type != WAD2Entry::mip_tex_type)
            continue;

        texs.push_back(
            entry.read_mip_tex(data).to_texture(data, this->palette));
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
