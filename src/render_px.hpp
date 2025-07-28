#pragma once

#include "sub_triangle.hpp"
#include "texture.hpp"

namespace RenderPixels {

void render_horizontal_line(int y, int x_0, int x_1, Frame &frame,
                            const SubTriangle &tri,
                            std::span<const Texture> texs);

}
