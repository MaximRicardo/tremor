#include "camera.hpp"
#include "angle.hpp"
#include "fov.hpp"
#include "input/input.hpp"
#include <algorithm>
#include <cmath>

Camera::Camera(Vec3 pos, Angle yaw, Angle pitch, Angle hfov)
    : pos(pos), yaw(yaw), pitch(pitch), hfov(hfov),
      vfov(FOV::horizontal_to_vertical(hfov))
{}

void Camera::limit_rotation()
{
    this->pitch = std::clamp(this->pitch, Angle(-85.f, Angle::Type::DEGREES),
                             Angle(85.f, Angle::Type::DEGREES));

    this->yaw.set(std::fmod(this->yaw.get(Angle::Type::DEGREES), 360.f),
                  Angle::Type::DEGREES);
}

void Camera::handle_input(float delta_time, Screen &screen)
{
    float mov_dist = delta_time;
    float turn_speed = delta_time;

    if (Input::is_key_down(Input::Key::W, screen))
        this->pos += this->move_forward_vec() * mov_dist;
    if (Input::is_key_down(Input::Key::S, screen))
        this->pos -= this->move_forward_vec() * mov_dist;
    if (Input::is_key_down(Input::Key::A, screen))
        this->pos -= this->move_right_vec() * mov_dist;
    if (Input::is_key_down(Input::Key::D, screen))
        this->pos += this->move_right_vec() * mov_dist;

    if (Input::is_key_down(Input::Key::Q, screen))
        this->pos.y -= mov_dist;
    if (Input::is_key_down(Input::Key::E, screen))
        this->pos.y += mov_dist;

    if (Input::is_key_down(Input::Key::UP, screen))
        this->pitch -= Angle(turn_speed);
    if (Input::is_key_down(Input::Key::DOWN, screen))
        this->pitch += Angle(turn_speed);
    if (Input::is_key_down(Input::Key::LEFT, screen))
        this->yaw += Angle(turn_speed);
    if (Input::is_key_down(Input::Key::RIGHT, screen))
        this->yaw -= Angle(turn_speed);

    this->limit_rotation();
}

Vec3 Camera::look_forward_vec() const
{
    Vec3 v = Vec3(0.f, 0.f, 1.f);
    v = v.rotate_about_x(this->pitch);
    v = v.rotate_about_y(this->yaw);
    return v;
}

Vec3 Camera::move_forward_vec() const
{
    Vec3 v = Vec3(0.f, 0.f, 1.f);
    v = v.rotate_about_y(this->yaw);
    return v;
}

Vec3 Camera::look_right_vec() const
{
    Vec3 v = Vec3(1.f, 0.f, 0.f);
    // rotating about x isn't necessary since the y and z components are 0 and
    // a rotation about x leaves the x component unmodified
    v = v.rotate_about_y(this->yaw);
    return v;
}

Vec3 Camera::move_right_vec() const
{
    Vec3 v = Vec3(1.f, 0.f, 0.f);
    v = v.rotate_about_y(this->yaw);
    return v;
}

Vec3 Camera::look_up_vec() const
{
    Vec3 v = Vec3(0.f, 1.f, 0.f);
    v = v.rotate_about_x(this->pitch);
    v = v.rotate_about_y(this->yaw);
    return v;
}

Vec3 Camera::move_up_vec() const
{
    Vec3 v = Vec3(0.f, 1.f, 0.f);
    v = v.rotate_about_y(this->yaw);
    return v;
}
