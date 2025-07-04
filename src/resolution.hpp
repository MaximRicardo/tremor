#pragma once

#include <cstddef>
#include <cstdint>

namespace Res {

constexpr uint32_t width = 320;
constexpr uint32_t height = 200;
constexpr size_t size = width * height;

constexpr uint32_t upscaled_width = 1280;
constexpr uint32_t upscaled_height = 720;
constexpr size_t upscaled_size = upscaled_width * upscaled_height;

} // namespace Res
