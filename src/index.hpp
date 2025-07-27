#pragma once

#include "ssize.hpp"
#include "vector/vec2.hpp"

namespace Index {

isize_t conv_2d_to_1d(Vec2i pos, isize_t width);
isize_t to_1d(Vec2i pos, isize_t width);

} // namespace Index
