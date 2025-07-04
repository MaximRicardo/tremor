#include "plane.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <tuple>

Plane::Plane(Vec3 normal, float d) : normal(normal), d(d) {}

namespace {

std::array<Vec3, 3> split_tri_1_in_front(const Plane &plane,
                                         const Vec3 &v_in_front,
                                         const Vec3 &v_behind_0,
                                         const Vec3 &v_behind_1)
{
    std::array<Vec3, 3> vs;

    float t;

    vs[0] = v_in_front;
    std::tie(vs[1], t) = plane.line_intersect_point(v_in_front, v_behind_0);
    std::tie(vs[2], t) = plane.line_intersect_point(v_in_front, v_behind_1);

    return vs;
}

std::array<Vec3, 3> split_tri_2_in_front_0(const Plane &plane,
                                           const Vec3 &v_in_front_0,
                                           const Vec3 &v_in_front_1,
                                           const Vec3 &v_behind_0)
{
    std::array<Vec3, 3> vs;

    float t;

    vs[0] = v_in_front_0;
    vs[1] = v_in_front_1;
    std::tie(vs[2], t) = plane.line_intersect_point(v_in_front_0, v_behind_0);

    return vs;
}

std::array<Vec3, 3> split_tri_2_in_front_1(const Plane &plane,
                                           const Vec3 &v_in_front_0,
                                           const Vec3 &v_in_front_1,
                                           const Vec3 &v_behind_0)
{
    std::array<Vec3, 3> vs;

    float t;

    vs[0] = v_in_front_1;
    std::tie(vs[1], t) = plane.line_intersect_point(v_in_front_1, v_behind_0);
    std::tie(vs[2], t) = plane.line_intersect_point(v_in_front_0, v_behind_0);

    return vs;
}

std::array<std::array<Vec3, 3>, 2>
split_tri_with_plane(const Plane &plane, const std::array<Vec3, 3> &vs,
                     const std::array<size_t, 3> &vs_in_front,
                     unsigned n_in_front,
                     const std::array<size_t, 3> &vs_behind, unsigned n_behind,
                     unsigned &n_sub_tris)
{
    assert(n_in_front + n_behind == 3);

    if (n_in_front == 0) {
        n_sub_tris = 0;
        return {};
    } else if (n_in_front == 1) {
        n_sub_tris = 1;
        return {split_tri_1_in_front(plane, vs[vs_in_front[0]],
                                     vs[vs_behind[0]], vs[vs_behind[1]])};
    } else if (n_in_front == 2) {
        n_sub_tris = 2;
        return {split_tri_2_in_front_0(plane, vs[vs_in_front[0]],
                                       vs[vs_in_front[1]], vs[vs_behind[0]]),
                split_tri_2_in_front_1(plane, vs[vs_in_front[0]],
                                       vs[vs_in_front[1]], vs[vs_behind[0]])};
    } else {
        n_sub_tris = 1;
        return {vs};
    }
}

} // namespace

std::tuple<Vec3, float> Plane::line_intersect_point(const Vec3 &start,
                                                    const Vec3 &end) const
{
    // Where a line intersects a plane
    float ad = start.dot(this->normal);
    float bd = end.dot(this->normal);
    float t = (this->d - ad) / (bd - ad);
    Vec3 line_start_to_end = end - start;
    Vec3 line_to_intersect = line_start_to_end * t;
    return std::make_tuple(start + line_to_intersect, t);
}

// there is no fucking way this is the best or even a good way to do this in c++
std::tuple<std::array<std::array<Vec3, 3>, 2>, unsigned>
Plane::clip(const std::array<Vec3, 3> &vs) const
{
    std::array<size_t, 3> vs_in_front;
    unsigned n_in_front = 0;
    std::array<size_t, 3> vs_behind;
    unsigned n_behind = 0;

    for (size_t i = 0; i < vs.size(); i++) {
        float v_d = normal.dot(vs[i]);

        if (v_d < this->d)
            vs_behind[n_behind++] = i;
        else
            vs_in_front[n_in_front++] = i;
    }

    unsigned n_sub_tris;
    auto tris = split_tri_with_plane(*this, vs, vs_in_front, n_in_front,
                                     vs_behind, n_behind, n_sub_tris);
    return std::make_tuple(tris, n_sub_tris);
}
