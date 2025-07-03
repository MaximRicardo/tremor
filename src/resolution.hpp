#pragma once

#include <cstdint>

namespace Res {
constexpr std::uint32_t width = 320;
constexpr std::uint32_t height = 320;
constexpr std::size_t size = width * height;

constexpr std::uint32_t upscaled_width = 1280;
constexpr std::uint32_t upscaled_height = 720;
constexpr std::size_t upscaled_size = upscaled_width * upscaled_height;
} // namespace Res
