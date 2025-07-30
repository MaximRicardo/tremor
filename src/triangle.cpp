#include "triangle.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "mat4x4.hpp"
#include "plane.hpp"
#include "sub_triangle.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

namespace {

// only accounts for y rotation rn
Vec3 world_v_to_camera(const Vec3 &v, const Camera &cam)
{
    Vec3 w = (v - cam.pos);
    // ORDER MATTERS
    w = w.rotate_about_y(-cam.yaw);
    w = w.rotate_about_x(-cam.pitch);

    return w;
}

// world space -> camera space
std::array<Vec3, 3> world_vs_to_camera(const std::array<Vec3, 3> &vs,
                                       const Camera &cam)
{
    std::array<Vec3, 3> result;

    for (size_t i = 0; i < vs.size(); ++i) {
        result[i] = world_v_to_camera(vs[i], cam);
    }

    return result;
}

// cam_vs is the camera-space vertices of the triangle to split.
Triangle::ProjectRet
split_tri_with_near_plane(const Triangle &tri,
                          const std::array<Vec3, 3> &cam_vs, const Camera &cam)
{
    Triangle::ProjectRet ret;

    Plane near_plane(Vec3(0.f, 0.f, 1.f), Consts::z_near);

    Triangle cam_space_tri(cam_vs, tri.vts, tri.tex_idx);

    Plane::ClipTriangleRet clip_ret;
    clip_ret = near_plane.clip(cam_space_tri);

    ret.n_sub_tris = clip_ret.n_tris;
    for (unsigned i = 0; i < clip_ret.n_tris; i++) {
        ret.sub_tris[i] =
            SubTriangle(clip_ret.tris[i].vs, clip_ret.tris[i].vts, &tri);
        ret.sub_tris[i].project_to_scr(cam);
    }

    return ret;
}

} // namespace

Triangle::Triangle() {};

Triangle::Triangle(std::array<Vec3, 3> vs, std::array<Vec2, 3> vts,
                   size_t tex_idx)
    : vs(vs), vts(vts), tex_idx(tex_idx)
{}

void Triangle::project(const Matrix4x4 &transform, const Camera &cam) const
{
    std::array<Vec3, 3> transf_vs;
    std::transform(
        this->vs.begin(), this->vs.end(), transf_vs.begin(),
        [transform](const Vec3 &v) { return transform * Vec4(v, 1.f); });
    auto cam_vs = world_vs_to_camera(transf_vs, cam);

    this->last_proj_ret = split_tri_with_near_plane(*this, cam_vs, cam);
}

void Triangle::render(const Matrix4x4 &transform, Frame &frame,
                      const Camera &cam,
                      const std::span<const Texture> texs) const
{
    this->project(transform, cam);

    for (unsigned i = 0; i < this->last_proj_ret.n_sub_tris; ++i) {
        this->last_proj_ret.sub_tris[i].render(frame, texs);
    }
}

bool Triangle::is_visible(const Matrix4x4 &transform, const Frame &frame,
                          const Camera &cam) const
{
    this->project(transform, cam);

    for (unsigned i = 0; i < this->last_proj_ret.n_sub_tris; ++i) {
        if (this->last_proj_ret.sub_tris[i].is_visible(frame))
            return true;
    }

    return false;
}

Plane Triangle::get_plane() const
{
    return Plane(this->vs);
}

Plane Triangle::get_plane(const Matrix4x4 &transform) const
{
    std::array<Vec3, 3> transf;
    std::transform(
        this->vs.begin(), this->vs.end(), transf.begin(),
        [transform](const Vec3 &v) { return transform * Vec4(v, 1.f); });

    return Plane(transf);
}

float Triangle::get_area() const
{
    Vec3 ab = this->vs[1] - this->vs[0];
    Vec3 ac = this->vs[2] - this->vs[0];

    return ab.cross(ac).length() / 2.f;
}

bool Triangle::is_degenerate() const
{
    return this->get_area() < Consts::epsilon;
}

bool Triangle::intersects(const Plane &plane) const
{
    bool v0 = plane.is_point_behind(this->vs[0]);
    bool v1 = plane.is_point_behind(this->vs[1]);
    bool v2 = plane.is_point_behind(this->vs[2]);
    return v0 != v1 || v0 != v2;
}

bool Triangle::is_on(const Plane &plane, float epsilon) const
{
    for (const auto &v : this->vs) {
        if (!plane.is_point_on(v, epsilon))
            return false;
    }

    return true;
}

bool Triangle::is_behind(const Plane &plane) const
{
    for (const auto &v : this->vs) {
        if (!plane.is_point_behind(v))
            return false;
    }

    return true;
}

bool Triangle::is_in_front(const Plane &plane) const
{
    for (const auto &v : this->vs) {
        if (!plane.is_point_in_front(v))
            return false;
    }

    return true;
}
