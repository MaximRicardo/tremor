#include "aabb.hpp"
#include <algorithm>
#include <array>

AABB::AABB() {};
AABB::AABB(Vec3 min, Vec3 max) : min(min), max(max) {}

bool AABB::is_point_inside(const Vec3 &p) const
{
    bool x_inside = min.x <= p.x && p.x <= max.x;
    bool y_inside = min.y <= p.y && p.y <= max.y;
    bool z_inside = min.z <= p.z && p.z <= max.z;
    return x_inside && y_inside && z_inside;
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
