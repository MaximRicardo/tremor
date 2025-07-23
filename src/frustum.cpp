#include "frustum.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <span>

Frustum::Frustum(std::span<const Plane, 6> planes)
{
    std::copy(planes.begin(), planes.end(), this->planes.begin());
}

Vec3 Frustum::forward_vec() const
{
    return this->planes[far_face].normal;
}

Vec3 Frustum::right_vec() const
{
    return this->planes[bottom_face]
        .normal.cross(this->planes[top_face].normal)
        .normalize();
}

Vec3 Frustum::up_vec() const
{
    return this->planes[right_face]
        .normal.cross(this->planes[left_face].normal)
        .normalize();
}

bool Frustum::contains(const Vec3 &p) const
{
    for (const auto &plane : this->planes) {
        if (!plane.is_point_behind(p))
            return false;
    }

    return true;
}

bool Frustum::maybe_partially_contains(const AABB &box) const
{
    for (const auto &plane : this->planes) {
        bool none_behind = true;
        for (const auto &v : box.get_vertices()) {
            if (plane.is_point_behind(v))
                none_behind = false;
        }

        if (none_behind)
            return false;
    }

    return true;
}
