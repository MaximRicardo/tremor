#pragma once

#include "../color.hpp"
#include "../vector/vec2.hpp"
#include "../vector/vec3.hpp"
#include <array>

class Triangle {

    std::array<Vec2, 3> scr_vs;

    void project();

public:
    std::array<Vec3, 3> vs;

    Triangle(Vec3 v_0, Vec3 v_1, Vec3 v_2);

    // positive if the triangle is counter-clockwise, negative otherwise
    float signed_area() const;
    bool point_inside(Vec2 &p) const;
    void render(Color *frame);
};
