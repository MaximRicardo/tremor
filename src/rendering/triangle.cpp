#include "triangle.hpp"
#include "../constants.hpp"
#include "sub_triangle.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>

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

Vec3 line_plane_intersect_point(Vec3 plane_pos, Vec3 plane_normal, Vec3 start,
                                Vec3 end, float &t)
{
    // Where a line intersects a plane
    float plane_d = -plane_normal.dot(plane_pos);
    float ad = start.dot(plane_normal);
    float bd = end.dot(plane_normal);
    t = (-plane_d - ad) / (bd - ad);
    Vec3 line_start_to_end = end - start;
    Vec3 line_to_intersect = line_start_to_end * t;
    return start + line_to_intersect;
}

std::array<Vec3, 3> split_tri_1_in_front(const Vec3 &v_in_front,
                                         const Vec3 v_behind_0,
                                         const Vec3 &v_behind_1)
{
    std::array<Vec3, 3> vs;

    float t;
    Vec3 plane_pos(0.f, 0.f, Consts::z_near);
    Vec3 plane_norm(0.f, 0.f, 1.f);

    vs[0] = v_in_front;
    vs[1] = line_plane_intersect_point(plane_pos, plane_norm, v_in_front,
                                       v_behind_0, t);
    vs[2] = line_plane_intersect_point(plane_pos, plane_norm, v_in_front,
                                       v_behind_1, t);

    return vs;
}

std::array<Vec3, 3> split_tri_2_in_front_0(const Vec3 &v_in_front_0,
                                           const Vec3 v_in_front_1,
                                           const Vec3 &v_behind_0)
{
    std::array<Vec3, 3> vs;

    float t;
    Vec3 plane_pos(0.f, 0.f, Consts::z_near);
    Vec3 plane_norm(0.f, 0.f, 1.f);

    vs[0] = v_in_front_0;
    vs[1] = v_in_front_1;
    vs[2] = line_plane_intersect_point(plane_pos, plane_norm, v_in_front_0,
                                       v_behind_0, t);

    return vs;
}

std::array<Vec3, 3> split_tri_2_in_front_1(const Vec3 &v_in_front_0,
                                           const Vec3 v_in_front_1,
                                           const Vec3 &v_behind_0)
{
    std::array<Vec3, 3> vs;

    float t;
    Vec3 plane_pos(0.f, 0.f, Consts::z_near);
    Vec3 plane_norm(0.f, 0.f, 1.f);

    vs[0] = v_in_front_1;
    vs[1] = line_plane_intersect_point(plane_pos, plane_norm, v_in_front_1,
                                       v_behind_0, t);
    vs[2] = line_plane_intersect_point(plane_pos, plane_norm, v_in_front_0,
                                       v_behind_0, t);

    return vs;
}

std::array<SubTriangle, 2> split_tri_with_near_plane(
    const std::array<Vec3, 3> &cam_vs, const std::array<size_t, 3> &vs_in_front,
    unsigned n_in_front, const std::array<size_t, 3> &vs_behind,
    unsigned n_behind, unsigned &n_sub_tris)
{
    assert(n_in_front + n_behind == 3);

    if (n_in_front == 0) {
        n_sub_tris = 0;
        return {SubTriangle({}), SubTriangle({})};
    } else if (n_in_front == 1) {
        n_sub_tris = 1;
        return {SubTriangle(split_tri_1_in_front(cam_vs[vs_in_front[0]],
                                                 cam_vs[vs_behind[0]],
                                                 cam_vs[vs_behind[1]])),
                SubTriangle({})};
    } else if (n_in_front == 2) {
        n_sub_tris = 2;
        return {SubTriangle(split_tri_2_in_front_0(cam_vs[vs_in_front[0]],
                                                   cam_vs[vs_in_front[1]],
                                                   cam_vs[vs_behind[0]])),
                SubTriangle(split_tri_2_in_front_1(cam_vs[vs_in_front[0]],
                                                   cam_vs[vs_in_front[1]],
                                                   cam_vs[vs_behind[0]]))};
    } else {
        n_sub_tris = 1;
        return {SubTriangle(cam_vs), SubTriangle({})};
    }
}

// cam_vs is the camera-space vertices of the triangle to split.
std::array<SubTriangle, 2>
split_tri_with_near_plane(const std::array<Vec3, 3> &cam_vs,
                          unsigned &n_sub_tris)
{
    std::array<size_t, 3> vs_in_front;
    unsigned n_in_front = 0;
    std::array<size_t, 3> vs_behind;
    unsigned n_behind = 0;

    for (size_t i = 0; i < cam_vs.size(); i++) {
        if (cam_vs[i].z < Consts::z_near)
            vs_behind[n_behind++] = i;
        else
            vs_in_front[n_in_front++] = i;
    }

    return split_tri_with_near_plane(cam_vs, vs_in_front, n_in_front, vs_behind,
                                     n_behind, n_sub_tris);
}

} // namespace

Triangle::Triangle(Vec3 v_0, Vec3 v_1, Vec3 v_2) : vs({v_0, v_1, v_2}) {}

std::array<SubTriangle, 2> Triangle::project(Camera &cam,
                                             unsigned &n_sub_tris) const
{
    auto cam_vs = world_vs_to_camera(this->vs, cam);

    auto sub_tris = split_tri_with_near_plane(cam_vs, n_sub_tris);
    for (unsigned i = 0; i < n_sub_tris; i++) {
        /*
        for (size_t j = 0; j < sub_tris[i].vs.size(); j++) {
            printf("tri[i].vs[j] = {%f, %f, %f}\n", sub_tris[i].vs[j].x,
                   sub_tris[i].vs[j].y, sub_tris[i].vs[j].z);
        }*/
        sub_tris[i].parent = this;
        sub_tris[i].project_to_scr();
    }

    return sub_tris;
}

void Triangle::render(Color *frame, Camera &cam)
{
    unsigned n_sub_tris;
    auto sub_tris = this->project(cam, n_sub_tris);

    for (unsigned i = 0; i < n_sub_tris; i++) {
        sub_tris[i].render(frame);
    }
}
