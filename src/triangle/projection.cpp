#include "../resolution.hpp"
#include "triangle.hpp"
#include <array>
#include <cstddef>
#include <cstdio>

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

    for (std::size_t i = 0; i < vs.size(); ++i) {
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

} // namespace

void Triangle::project()
{
    auto norm_scr_vs = camera_vs_to_norm_scr(this->vs);
    this->scr_vs = norm_scr_vs_to_scr(norm_scr_vs);
}
