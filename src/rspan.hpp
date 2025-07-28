#pragma once

#include "frame.hpp"
#include "texture.hpp"
#include <span>

namespace RSpan {

constexpr bool enabled = false;

// DOES NOT WORK RN!
void render(Frame &frame, std::span<const Texture> texs);

} // namespace RSpan
