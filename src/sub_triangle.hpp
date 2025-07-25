#pragma once

#include "frame.hpp"
#include "texture.hpp"
#include "vector/vec2.hpp"
#include "vector/vec3.hpp"
#include <array>
#include <span>

class Triangle;
class Camera;

// used exclusively for rendering.
// when a triangle is clipped with the near plane, 0-2 instances of SubTriangle
// will be created and rendered.
class SubTriangle {

    std::array<Vec2i, 3> screen_vs;

    SubTriangle() = default;

public:
    // IN CAMERA SPACE
    std::array<Vec3, 3> vs;
    std::array<Vec2, 3> vts;

    const Triangle *parent = nullptr;

    SubTriangle(std::span<const Vec3, 3> vs, std::span<const Vec2, 3> vts,
                const Triangle *parent = nullptr);

    std::array<Vec2i, 3> get_screen_vs() const;
    void project_to_scr(const Camera &cam);
    void render(Frame &frame, std::span<const Texture> texs);
    // goes up to 1 for the whole texture
    float tex_space_area() const;

    friend Triangle;
};
