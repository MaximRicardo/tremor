#include "aabb.hpp"
#include "constants.hpp"
#include "line.hpp"
#include "resolution.hpp"
#include "vector/vec3.hpp"
#include <algorithm>
#include <array>
#include <cassert>

AABB::AABB(Vec3 min, Vec3 max) : min(min), max(max) {}

AABB AABB::map_box()
{
    return AABB(Vec3(Consts::map_bounding_box_min, Consts::map_bounding_box_min,
                     Consts::map_bounding_box_min),
                Vec3(Consts::map_bounding_box_max, Consts::map_bounding_box_max,
                     Consts::map_bounding_box_max));
}

bool AABB::contains(const Vec3 &p) const
{
    bool x_inside =
        min.x - Consts::epsilon <= p.x && p.x <= max.x + Consts::epsilon;
    bool y_inside =
        min.y - Consts::epsilon <= p.y && p.y <= max.y + Consts::epsilon;
    bool z_inside =
        min.z - Consts::epsilon <= p.z && p.z <= max.z + Consts::epsilon;
    return x_inside && y_inside && z_inside;
}

bool AABB::contains(const AABB &box) const
{
    return this->contains(box.min) && this->contains(box.max);
}

bool AABB::partially_contains(const AABB &box) const
{
    Line1D this_x(this->min.x, this->max.x);
    Line1D this_y(this->min.y, this->max.y);
    Line1D this_z(this->min.z, this->max.z);

    Line1D other_x(box.min.x, box.max.x);
    Line1D other_y(box.min.y, box.max.y);
    Line1D other_z(box.min.z, box.max.z);

    return this_x.partially_contains(other_x) &&
           this_y.partially_contains(other_y) &&
           this_z.partially_contains(other_z);
}

std::array<Vec3, 8> AABB::get_vertices() const
{
    return {
        Vec3(this->max.x, this->min.y, this->min.z),
        Vec3(this->min.x, this->min.y, this->min.z),
        Vec3(this->min.x, this->min.y, this->max.z),
        Vec3(this->max.x, this->min.y, this->max.z),

        Vec3(this->max.x, this->max.y, this->max.z),
        Vec3(this->min.x, this->max.y, this->max.z),
        Vec3(this->min.x, this->max.y, this->min.z),
        Vec3(this->max.x, this->max.y, this->min.z),
    };
}

std::vector<Vec3> AABB::clipped_vertices(const Plane &plane) const
{
    auto vs = this->get_vertices();

    std::vector<Vec3> new_vs;
    Vec3 prev_v = vs.back();
    for (const auto &v : vs) {
        if (plane.does_line_intersect(prev_v, v))
            new_vs.push_back(
                std::get<0>(plane.line_intersect_point(prev_v, v)));

        if (!plane.is_point_behind(v))
            new_vs.push_back(v);

        prev_v = v;
    }

    return new_vs;
}

void AABB::clip(const Plane &plane)
{
    auto clipped_vs = this->clipped_vertices(plane);
    if (clipped_vs.size() == 0) {
        this->min = Vec3(0.f, 0.f, 0.f);
        this->max = Vec3(0.f, 0.f, 0.f);
        return;
    }

    Vec3 new_min = Vec3(100000.f, 100000.f, 100000.f);
    Vec3 new_max = Vec3(-100000.f, -100000.f, -100000.f);
    for (const auto &p : clipped_vs) {
        this->merge(p);
    }

    this->min = new_min;
    this->max = new_max;
}

bool AABB::intersects(const Plane &plane) const
{
    return !this->intersection_points(plane).empty();
}

std::vector<Vec3> AABB::intersection_points(const Plane &plane) const
{
    auto vs = this->get_vertices();

    std::vector<Vec3> intersections;
    Vec3 prev_v = vs.back();
    for (const auto &v : vs) {
        if (plane.does_line_intersect(prev_v, v))
            intersections.push_back(
                std::get<0>(plane.line_intersect_point(prev_v, v)));

        prev_v = v;
    }

    return intersections;
}

void AABB::merge(const Vec3 &p)
{
    this->min.x = std::min(this->min.x, p.x);
    this->min.y = std::min(this->min.y, p.y);
    this->min.z = std::min(this->min.z, p.z);

    this->max.x = std::max(this->max.x, p.x);
    this->max.y = std::max(this->max.y, p.y);
    this->max.z = std::max(this->max.z, p.z);
}

void AABB::merge(const AABB &other)
{
    this->merge(other.min);
    this->merge(other.max);
}

Vec3 AABB::get_center() const
{
    return this->min.mix(this->max, 0.5f);
}

AABB2D::AABB2D(Vec2 min, Vec2 max) : min(min), max(max) {}

AABB2Di::AABB2Di(Vec2i min, Vec2i max) : min(min), max(max) {}

bool AABB2Di::is_on_screen() const
{
    bool x_off = this->max.x < 0 || this->min.x >= Res::width;
    bool y_off = this->max.y < 0 || this->min.y >= Res::height;
    return !x_off && !y_off;
}
