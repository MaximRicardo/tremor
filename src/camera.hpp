#pragma once

#include "angle.hpp"
#include "screen/screen.hpp"
#include "vector/vec3.hpp"

class Camera {

    // limits pitch to +-85 deg, and makes yaw wrap around past +-360 deg
    void limit_rotation();

public:
    Vec3 pos;
    // pitch is inverted, higher pitch means rotating further down
    Angle yaw, pitch;
    Angle hfov, vfov;

    Camera(Vec3 pos, Angle yaw, Angle pitch, Angle hfov = Angle(0.f));

    void handle_input(float delta_time, Screen &screen);

    // the view directions of the camera
    Vec3 look_forward_vec() const;
    Vec3 look_right_vec() const;
    Vec3 look_up_vec() const;

    // the movement directions of the camera. these don't account for pitch, cuz
    // the player can't start flying upwards and shit.
    Vec3 move_forward_vec() const;
    Vec3 move_right_vec() const;
    Vec3 move_up_vec() const;
};
