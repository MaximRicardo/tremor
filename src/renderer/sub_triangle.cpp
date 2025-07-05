#include "sub_triangle.hpp"
#include "../resolution.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <tuple>

namespace {

Vec2 camera_v_to_norm_scr(const Vec3 &v)
{
    Vec2 w;

    w.x = v.x / v.z;
    w.y = v.y / v.z;

    return w;
}

// camera space -> normalized screen space
std::array<Vec2, 3> camera_vs_to_norm_scr(const std::array<Vec3, 3> &vs)
{
    std::array<Vec2, 3> result;

    for (size_t i = 0; i < vs.size(); ++i) {
        result[i] = camera_v_to_norm_scr(vs[i]);
    }

    return result;
}

Vec2 norm_scr_v_to_scr(const Vec2 &v)
{
    Vec2 w;

    w.x = (v.x + 1.f) / 2.f * Res::width;
    w.y = (-v.y + 1.f) / 2.f * Res::height;

    return w;
}

// normalized screen space -> screen space
std::array<Vec2, 3> norm_scr_vs_to_scr(const std::array<Vec2, 3> &vs)
{
    std::array<Vec2, 3> result;

    for (std::size_t i = 0; i < vs.size(); i++) {
        result[i] = norm_scr_v_to_scr(vs[i]);
    }

    return result;
}

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

SubTriangle::SubTriangle(std::array<Vec3, 3> vs, const Triangle *parent)
    : vs(vs), parent(parent)
{}

std::array<Vec2, 3> SubTriangle::get_screen_vs() const
{
    return this->screen_vs;
}

void SubTriangle::project_to_scr()
{
    auto norm_scr_vs = camera_vs_to_norm_scr(this->vs);
    this->screen_vs = norm_scr_vs_to_scr(norm_scr_vs);
}

// positive if the triangle is counter-clockwise, negative otherwise
float SubTriangle::signed_area() const
{
    return -0.5f * (-this->screen_vs[1].y * this->screen_vs[2].x +
                    this->screen_vs[0].y *
                        (-this->screen_vs[1].x + this->screen_vs[2].x) +
                    this->screen_vs[0].x *
                        (this->screen_vs[1].y - this->screen_vs[2].y) +
                    this->screen_vs[1].x * this->screen_vs[2].y);
}

bool SubTriangle::point_inside(Vec2 &p) const
{
    // barycentric coordinates are used
    float s = 1.f / (2.f * -this->signed_area()) *
              (this->screen_vs[0].y * this->screen_vs[2].x -
               this->screen_vs[0].x * this->screen_vs[2].y +
               (this->screen_vs[2].y - this->screen_vs[0].y) * p.x +
               (this->screen_vs[0].x - this->screen_vs[2].x) * p.y);
    float t = 1.f / (2.f * -this->signed_area()) *
              (this->screen_vs[0].x * this->screen_vs[1].y -
               this->screen_vs[0].y * this->screen_vs[1].x +
               (this->screen_vs[0].y - this->screen_vs[1].y) * p.x +
               (this->screen_vs[1].x - this->screen_vs[0].x) * p.y);

    bool s_in_range = 0.f <= s && s <= 1.f;
    bool t_in_range = 0.f <= t && t <= 1.f;
    bool total_in_range = s + t <= 1.f;

    return s_in_range && t_in_range && total_in_range;
}

void SubTriangle::render(Color *frame)
{
    float x_min, x_max;
    std::tie(x_min, x_max) = std::minmax(
        {this->screen_vs[0].x, this->screen_vs[1].x, this->screen_vs[2].x});

    float y_min, y_max;
    std::tie(y_min, y_max) = std::minmax(
        {this->screen_vs[0].y, this->screen_vs[1].y, this->screen_vs[2].y});

    if (x_max < 0 || x_min >= Res::width)
        return;
    if (y_max < 0 || y_min >= Res::height)
        return;

    x_min = std::max(x_min, 0.f);
    x_max = std::min(x_max, static_cast<float>(Res::width - 1));

    for (int32_t y = std::floor(y_min); y < std::ceil(y_max); ++y) {
        for (int32_t x = std::floor(x_min); x < std::ceil(x_max); ++x) {
            if (!on_screen(x, y))
                continue;

            Vec2 p(x, y);
            if (!this->point_inside(p))
                continue;

            frame[scr_2d_to_1d(x, y)] = Color(0, 255, 0);
        }
    }
}
