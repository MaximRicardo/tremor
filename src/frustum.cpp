#include "frustum.hpp"
#include <algorithm>
#include <span>

Frustum::Frustum(std::span<const Plane, 6> planes)
{
    std::copy(planes.begin(), planes.end(), this->planes.begin());
}

bool Frustum::point_inside(const Vec3 &p) const
{
    for (const auto &plane : this->planes) {
        if (!plane.is_point_behind(p))
            return false;
    }

    return true;
}

bool Frustum::b_box_has_vertex_inside(const AABB &box) const
{
    auto vs = box.get_vertices();

    for (const auto &v : vs) {
        if (!this->point_inside(v))
            return false;
    }

    return true;
}

bool Frustum::b_box_intersects(const AABB &box) const
{
    for (const auto &plane : this->planes) {
        if (box.intersects(plane))
            return true;
    }

    return false;
}

bool Frustum::b_box_partially_inside(const AABB &box) const
{
    return this->b_box_has_vertex_inside(box) || this->b_box_intersects(box);
}
