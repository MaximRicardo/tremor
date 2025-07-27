#include "index.hpp"
#include "ssize.hpp"

isize_t Index::conv_2d_to_1d(Vec2i pos, isize_t width)
{
    return pos.y * width + pos.x;
}

isize_t Index::to_1d(Vec2i pos, isize_t width)
{
    return pos.y * width + pos.x;
}
