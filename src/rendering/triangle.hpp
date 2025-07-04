#pragma once

#include "../camera.hpp"
#include "../color.hpp"
#include "../vector/vec2.hpp"
#include "../vector/vec3.hpp"
#include <array>

class Triangle {

    // returns this->vs in pixel screen space
    std::array<Vec2, 3> project(Camera &cam) const;

public:
    std::array<Vec3, 3> vs;

    Triangle(Vec3 v_0, Vec3 v_1, Vec3 v_2);

    void render(Color *frame, Camera &cam);
};
