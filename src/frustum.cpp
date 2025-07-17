#include "frustum.hpp"
#include <algorithm>
#include <cassert>
#include <span>

namespace {

Vec3 box_minmax(const AABB &box, bool get_min)
{
    return get_min ? box.min : box.max;
}

} // namespace

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

// function from https://iquilezles.org/articles/frustumcorrect/
/*
bool Frustum::b_box_partially_inside(const AABB &box) const
{
    assert(this->planes.size() == 6);

    for (const auto &plane : this->planes) {
        int out = 0;

        auto vs = box.get_vertices();
        for (const auto &v : vs) {
            out += plane.normal.dot(v) < plane.d;
        }

        if (out == 8)
            return false;
    }

    return true;
}
*/

bool Frustum::b_box_partially_inside(const AABB &box) const
{
    for (const auto &plane : this->planes) {
        bool nx = plane.normal.x > 0.f;
        bool ny = plane.normal.y > 0.f;
        bool nz = plane.normal.z > 0.f;

        float dot = (plane.normal.x * box_minmax(box, nx).x) +
                    (plane.normal.y * box_minmax(box, ny).y) +
                    (plane.normal.z * box_minmax(box, nz).z);

        if (dot > plane.d)
            return false;
    }

    return true;
}
