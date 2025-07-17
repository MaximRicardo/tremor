#include "triangle.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "plane.hpp"
#include "sub_triangle.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>

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

Triangle::ProjectRet Triangle::project(const Camera &cam) const
{
    auto cam_vs = world_vs_to_camera(this->vs, cam);

    return split_tri_with_near_plane(*this, cam_vs, cam);
}

void Triangle::render(std::span<Color> frame, std::span<float> depth_buffer,
                      const Camera &cam,
                      const std::span<const Texture> texs) const
{
    Triangle::ProjectRet project_ret;
    project_ret = this->project(cam);

    for (unsigned i = 0; i < project_ret.n_sub_tris; i++) {
        project_ret.sub_tris[i].render(frame, depth_buffer, texs);
    }
}

Plane Triangle::get_plane() const
{
    Vec3 a = this->vs[1] - this->vs[0];
    Vec3 b = this->vs[2] - this->vs[0];

    Vec3 normal = b.cross(a).normalize();
    float d = normal.dot(this->vs[0]);

    return Plane(normal, d);
}
