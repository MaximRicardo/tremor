#include "transform.hpp"
#include "resolution.hpp"
#include <cmath>

Vec3 Transform::world_space_to_cam_space(const Vec3 &v, const Camera &cam)
{
    Vec3 w = (v - cam.pos);
    // ORDER MATTERS
    w = w.rotate_about_y(-cam.yaw);
    w = w.rotate_about_x(-cam.pitch);

    return w;
}

Vec2 Transform::cam_space_to_norm_scr(const Vec3 &v, const Camera &cam)
{
    float x_fov_mult = 1.f / std::tan(cam.hfov.get() / 2.f);
    float y_fov_mult = 1.f / std::tan(cam.vfov.get() / 2.f);

    Vec2 w;

    w.x = v.x * x_fov_mult / v.z;
    w.y = v.y * y_fov_mult / v.z;

    return w;
}

Vec2i Transform::norm_scr_to_scr_space(const Vec2 &v)
{
    Vec2i w;
    w.x = std::round((v.x + 1.f) / 2.f * Res::width);
    w.y = std::round((-v.y + 1.f) / 2.f * Res::height);

    return w;
}
