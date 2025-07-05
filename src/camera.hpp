#pragma once

#include "screen/screen.hpp"
#include "vector/vec3.hpp"

class Camera {

public:
    Vec3 pos;
    Vec3 rot;

    Camera(Vec3 pos = Vec3(0.f, 0.f, 0.f), Vec3 rot = Vec3(0.f, 0.f, 0.f));

    void handle_input(float delta_time, Screen &screen);
};
