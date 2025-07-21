#pragma once

namespace Consts {

constexpr float epsilon = 0.001f;

constexpr float z_near = 0.1f;
constexpr float z_far = 1000.f;

constexpr float pi = 3.141592f;
constexpr float deg_2_rad_mul = pi / 180.f;
constexpr float rad_2_deg_mul = 1.f / deg_2_rad_mul;

constexpr float map_bounding_box_min_x = -10000.f;
constexpr float map_bounding_box_min_y = -10000.f;
constexpr float map_bounding_box_min_z = -10000.f;
constexpr float map_bounding_box_max_x = 10000.f;
constexpr float map_bounding_box_max_y = 10000.f;
constexpr float map_bounding_box_max_z = 10000.f;

} // namespace Consts
