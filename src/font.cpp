#include "font.hpp"
#include <cstdint>

bool Font::active_pixel(char c, Vec2i pos)
{
    auto &bitmap = bitmaps[c];

    int32_t mir_x = char_width - pos.x - 1;
    return (bitmap[pos.y] >> mir_x) & 0x1;
}
