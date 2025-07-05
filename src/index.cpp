#include "index.hpp"

size_t Index::conv_2d_to_1d(Vec2i pos, size_t width)
{
    return pos.y * width + pos.x;
}
