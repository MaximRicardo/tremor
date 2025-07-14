#pragma once

#include "angle.hpp"
#include "screen/screen.hpp"
#include "vector/vec3.hpp"

class Camera {

public:
    Vec3 pos;
    EulerAngle rot;
    Angle hfov, vfov;

    Camera(Vec3 pos, EulerAngle rot, Angle hfov = Angle(0.f));

    void handle_input(float delta_time, Screen &screen);
};
