#pragma once

#include <cstddef>
#include <cstdint>

namespace Res {

inline int32_t width = 320;
inline int32_t height = 200;

size_t n_pixels();
float aspect_ratio();

inline int32_t upscaled_width = 1280;
inline int32_t upscaled_height = 800;
size_t upscaled_n_pixels();

} // namespace Res
