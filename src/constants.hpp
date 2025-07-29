#pragma once

namespace Consts {

constexpr float epsilon = 0.005f;

constexpr float z_near = 1.f;
constexpr float z_far = 4096.f;

constexpr float pi = 3.141592f;
constexpr float deg_2_rad_mul = pi / 180.f;
constexpr float rad_2_deg_mul = 1.f / deg_2_rad_mul;

constexpr float map_bounding_box_min = -10000.f;
constexpr float map_bounding_box_max = 10000.f;

} // namespace Consts
