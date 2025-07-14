#pragma once

#include "camera.hpp"
#include "color.hpp"
#include "texture.hpp"
#include "vector/vec2.hpp"
#include "vector/vec3.hpp"
#include <array>
#include <span>

class Triangle;

// used exclusively for rendering.
// when a triangle is clipped with the near plane, 0-2 instances of SubTriangle
// will be created and rendered.
class SubTriangle {

    std::array<Vec2i, 3> screen_vs;

public:
    // IN CAMERA SPACE
    std::array<Vec3, 3> vs;
    std::array<Vec2, 3> vts;

    const Triangle *parent;

    SubTriangle(std::array<Vec3, 3> vs, std::array<Vec2, 3> vts,
                const Triangle *parent = nullptr);

    std::array<Vec2i, 3> get_screen_vs() const;
    void project_to_scr(const Camera &cam);
    void render(std::span<Color> frame, std::span<float> depth_buffer,
                std::span<const Texture> texs);
};
