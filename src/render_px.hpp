#pragma once

#include "sub_triangle.hpp"
#include "texture.hpp"
#include <cstdint>

namespace RenderPixels {

void render_horizontal_line(int32_t y, int32_t x_0, int32_t x_1, Frame &frame,
                            const SubTriangle &tri,
                            std::span<const Texture> texs);
bool horizontal_line_visible(int32_t y, int32_t x_0, int32_t x_1,
                             const Frame &frame, const SubTriangle &tri);

} // namespace RenderPixels
