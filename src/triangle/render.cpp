#include "../resolution.hpp"
#include "triangle.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <tuple>

namespace {

bool on_screen(uint32_t x, uint32_t y)
{
    bool x_in_range = x < Res::width;
    bool y_in_range = y < Res::height;
    return x_in_range && y_in_range;
}

size_t scr_2d_to_1d(size_t x, size_t y)
{
    return Res::width * y + x;
}

} // namespace

void Triangle::render(Color *frame)
{
    // backface culling
    if (this->signed_area() < 0.f)
        return;

    float x_min, x_max;
    std::tie(x_min, x_max) =
        std::minmax({this->scr_vs[0].x, this->scr_vs[1].x, this->scr_vs[2].x});

    float y_min, y_max;
    std::tie(y_min, y_max) =
        std::minmax({this->scr_vs[0].y, this->scr_vs[1].y, this->scr_vs[2].y});

    for (int32_t y = std::floor(y_min); y < std::ceil(y_max); y++) {
        for (int32_t x = std::floor(x_min); x < std::ceil(x_max); x++) {
            if (!on_screen(x, y))
                continue;

            Vec2 p(x, y);
            if (!this->point_inside(p))
                continue;

            frame[scr_2d_to_1d(x, y)] = Color(0, 255, 0);
        }
    }
}
