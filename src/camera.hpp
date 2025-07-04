#pragma once

#include "screen/screen.hpp"
#include "vector/vec3.hpp"

class Camera {

public:
    Vec3 pos;

    Camera(Vec3 pos);

    void handle_input(float delta_time, Screen &screen);
};
