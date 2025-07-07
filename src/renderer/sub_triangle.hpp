#pragma once

#include "../color.hpp"
#include "../texture.hpp"
#include "../vector/vec2.hpp"
#include "../vector/vec3.hpp"
#include <array>

class Triangle;

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
    void project_to_scr();
    void render(Color *frame, float *depth_buffer, const Texture *texs);
};
