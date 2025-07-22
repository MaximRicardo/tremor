#pragma once

#include "../texture.hpp"
#include "wad.hpp"
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace WAD {

constexpr std::string wad2_format_name = "WAD2";
constexpr size_t wad2_max_tex_name = 16;

class WAD2MipHeader {

public:
    // should be the same as the texture's corresponding entry's name
    std::string name;
    int32_t width;
    int32_t height;

    // ok, so it seems like these point to the location of the specific mipmap
    // level's info, subtracted by 4, subtracted by own_offset.
    // so scale_1_pos = mipmap_texel_data_offset - 4 - own_offset
    // if ur wondering why tf this is, dw, i'm wondering the exact same thing.
    int32_t scale_1_pos;
    int32_t scale_2_pos;
    int32_t scale_4_pos;
    int32_t scale_8_pos;

    // offset of the header itself
    int32_t own_offset;

    static constexpr size_t name_offset = 0;
    static constexpr size_t width_offset = 16;
    static constexpr size_t height_offset = 20;
    static constexpr size_t scale_1_pos_offset = 24;
    static constexpr size_t scale_2_pos_offset = 28;
    static constexpr size_t scale_4_pos_offset = 32;
    static constexpr size_t scale_8_pos_offset = 36;

    WAD2MipHeader(std::span<const uint8_t> data, size_t offset);

    std::vector<uint8_t> get_pixels(std::span<const uint8_t> data,
                                    unsigned mipmap_lvl) const;
    Texture to_texture(std::span<const uint8_t> data) const;
};

class WAD2Header : public Header {

public:
    WAD2Header(std::span<const uint8_t> data, size_t offset);
};

class WAD2Entry {

public:
    int32_t offset; // offset in WAD file
    int32_t d_size; // size in file
    int32_t size;   // uncompressed size
    int8_t type;
    bool is_comprsd; // only takes up a single byte in the file
    // unused int16_t here between compression and name in the file
    std::string name; // up to wad2_max_tex_name length (including '\0')

    // offsets of each member in the file, from the base of the entry
    static constexpr size_t offset_offset = 0; // lol
    static constexpr size_t d_size_offset = 4;
    static constexpr size_t size_offset = 8;
    static constexpr size_t type_offset = 12;
    static constexpr size_t is_comprsd_offset = 13;
    static constexpr size_t unused_offset = 14;
    static constexpr size_t name_offset = 16;

    // each entry takes up exactly 32 bytes in the file
    static constexpr size_t entry_offset_inc = 32;

    // meaning of the values type can have
    static constexpr int8_t color_palette_type = '@'; // not supported btw
    static constexpr int8_t status_bar_type = 'B';    // pics for status bar
    static constexpr int8_t mip_tex_type = 'D'; // honestly not sure yet, i'm
                                                // guessing a tex to which
                                                // should be mipmapped?
    static constexpr int8_t console_pic_type = 'E'; // idk

    WAD2Entry(std::span<const uint8_t> data, size_t offset);

    // ASSUMES THE ENTRY IS A MIP TEX
    WAD2MipHeader read_mip_tex(std::span<const uint8_t> data) const;
};

class WAD2Dir {

public:
    std::vector<WAD2Entry> entries;

    WAD2Dir(const WAD2Header &header, std::span<const uint8_t> data);

    std::vector<Texture> get_textures(std::span<const uint8_t> data) const;
};

std::vector<Texture> load_wad2(std::span<const uint8_t> data);

} // namespace WAD
