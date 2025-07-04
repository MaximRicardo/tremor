#include "triangle.hpp"
#include "../resolution.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <tuple>

namespace {

Vec3 world_v_to_camera(const Vec3 &v, Camera &cam)
{
    Vec3 w = v - cam.pos;

    return w;
}

// world space -> camera space
std::array<Vec3, 3> world_vs_to_camera(const std::array<Vec3, 3> &vs,
                                       Camera &cam)
{
    std::array<Vec3, 3> result;

    for (size_t i = 0; i < vs.size(); ++i) {
        result[i] = world_v_to_camera(vs[i], cam);
    }

    return result;
}

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

// positive if the triangle is counter-clockwise, negative otherwise
float signed_triangle_area(std::array<Vec2, 3> &vs)
{
    return -0.5f * (-vs[1].y * vs[2].x + vs[0].y * (-vs[1].x + vs[2].x) +
                    vs[0].x * (vs[1].y - vs[2].y) + vs[1].x * vs[2].y);
}

bool point_inside_triangle(Vec2 &p, std::array<Vec2, 3> &vs)
{
    float area = signed_triangle_area(vs);

    // barycentric coordinates are used
    float s = 1.f / (2.f * -area) *
              (vs[0].y * vs[2].x - vs[0].x * vs[2].y +
               (vs[2].y - vs[0].y) * p.x + (vs[0].x - vs[2].x) * p.y);
    float t = 1.f / (2.f * -area) *
              (vs[0].x * vs[1].y - vs[0].y * vs[1].x +
               (vs[1].y - vs[1].y) * p.x + (vs[1].x - vs[0].x) * p.y);

    bool s_in_range = 0.f <= s && s <= 1.f;
    bool t_in_range = 0.f <= t && t <= 1.f;
    bool total_in_range = s + t <= 1.f;

    return s_in_range && t_in_range && total_in_range;
}

} // namespace

Triangle::Triangle(Vec3 v_0, Vec3 v_1, Vec3 v_2) : vs({v_0, v_1, v_2}) {}

std::array<Vec2, 3> Triangle::project(Camera &cam) const
{
    auto cam_vs = world_vs_to_camera(this->vs, cam);
    auto norm_scr_vs = camera_vs_to_norm_scr(cam_vs);
    return norm_scr_vs_to_scr(norm_scr_vs);
}

void Triangle::render(Color *frame, Camera &cam)
{
    auto scr_vs = this->project(cam);

    // backface culling
    if (signed_triangle_area(scr_vs) < 0.f)
        return;

    float x_min, x_max;
    std::tie(x_min, x_max) =
        std::minmax({scr_vs[0].x, scr_vs[1].x, scr_vs[2].x});

    float y_min, y_max;
    std::tie(y_min, y_max) =
        std::minmax({scr_vs[0].y, scr_vs[1].y, scr_vs[2].y});

    for (int32_t y = std::floor(y_min); y < std::ceil(y_max); ++y) {
        for (int32_t x = std::floor(x_min); x < std::ceil(x_max); ++x) {
            if (!on_screen(x, y))
                continue;

            Vec2 p(x, y);
            if (!point_inside_triangle(p, scr_vs))
                continue;

            frame[scr_2d_to_1d(x, y)] = Color(0, 255, 0);
        }
    }
}
