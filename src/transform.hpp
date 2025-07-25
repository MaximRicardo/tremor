#pragma once

#include "camera.hpp"

namespace Transform {

Vec3 world_space_to_cam_space(const Vec3 &v, const Camera &cam);
Vec2 cam_space_to_norm_scr(const Vec3 &v, const Camera &cam);
Vec2i norm_scr_to_scr_space(const Vec2 &v);

} // namespace Transform
