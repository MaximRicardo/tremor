#include "aabb.hpp"
#include "constants.hpp"
#include <algorithm>
#include <array>
#include <cassert>

namespace {

class Line1D {

public:
    float min_x, max_x;

    Line1D(float min_x, float max_x) : min_x(min_x), max_x(max_x) {}

    bool partially_contains(const Line1D &other) const;
};

bool Line1D::partially_contains(const Line1D &other) const
{
    return this->max_x >= other.min_x - Consts::epsilon &&
           other.max_x >= this->min_x - Consts::epsilon;
}

} // namespace

AABB::AABB() {};
AABB::AABB(Vec3 min, Vec3 max) : min(min), max(max) {}

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
        new_min.x = std::min(new_min.x, p.x);
        new_min.y = std::min(new_min.y, p.y);
        new_min.z = std::min(new_min.z, p.z);

        new_max.x = std::max(new_max.x, p.x);
        new_max.y = std::max(new_max.y, p.y);
        new_max.z = std::max(new_max.z, p.z);
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
