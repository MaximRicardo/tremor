#include "triangle.hpp"
#include "../constants.hpp"
#include "plane.hpp"
#include "sub_triangle.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
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

// cam_vs is the camera-space vertices of the triangle to split.
std::tuple<std::array<SubTriangle, 2>, unsigned>
split_tri_with_near_plane(const Triangle &tri,
                          const std::array<Vec3, 3> &cam_vs)
{
    std::array<SubTriangle, 2> sub_tris;

    Plane near_plane(Vec3(0.f, 0.f, 1.f), Consts::z_near);

    std::array<std::array<Vec3, 3>, 2> sub_tris_vs;
    unsigned n_sub_tris;
    std::tie(sub_tris_vs, n_sub_tris) = near_plane.clip(cam_vs);

    for (unsigned i = 0; i < n_sub_tris; i++) {
        sub_tris[i] = SubTriangle(sub_tris_vs[i], &tri);
        sub_tris[i].project_to_scr();
    }

    return std::make_tuple(sub_tris, n_sub_tris);
}

} // namespace

Triangle::Triangle(Vec3 v_0, Vec3 v_1, Vec3 v_2) : vs({v_0, v_1, v_2}) {}

std::tuple<std::array<SubTriangle, 2>, unsigned>
Triangle::project(Camera &cam) const
{
    auto cam_vs = world_vs_to_camera(this->vs, cam);

    return split_tri_with_near_plane(*this, cam_vs);
}

void Triangle::render(Color *frame, Camera &cam)
{
    unsigned n_sub_tris;
    std::array<SubTriangle, 2> sub_tris;
    std::tie(sub_tris, n_sub_tris) = this->project(cam);

    for (unsigned i = 0; i < n_sub_tris; i++) {
        sub_tris[i].render(frame);
    }
}
