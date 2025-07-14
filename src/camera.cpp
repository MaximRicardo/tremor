#include "camera.hpp"
#include "angle.hpp"
#include "fov.hpp"
#include "input/input.hpp"
#include <iostream>

Camera::Camera(Vec3 pos, EulerAngle rot, Angle hfov)
    : pos(pos), rot(rot), hfov(hfov), vfov(FOV::horizontal_to_vertical(hfov))
{}

void Camera::handle_input(float delta_time, Screen &screen)
{
    float mov_dist = delta_time;

    std::cout << this->forward().x << ", " << this->forward().y << ", "
              << this->forward().z << '\n';

    if (Input::is_key_down(Input::Key::W, screen))
        this->pos += this->forward() * mov_dist;
    if (Input::is_key_down(Input::Key::S, screen))
        this->pos -= this->forward() * mov_dist;
    if (Input::is_key_down(Input::Key::A, screen))
        this->pos -= this->right() * mov_dist;
    if (Input::is_key_down(Input::Key::D, screen))
        this->pos += this->right() * mov_dist;

    if (Input::is_key_down(Input::Key::Q, screen))
        this->pos.y -= mov_dist;
    if (Input::is_key_down(Input::Key::E, screen))
        this->pos.y += mov_dist;

    if (Input::is_key_down(Input::Key::LEFT, screen))
        this->rot.y += Angle(delta_time);
    if (Input::is_key_down(Input::Key::RIGHT, screen))
        this->rot.y -= Angle(delta_time);
}

Vec3 Camera::forward() const
{
    Vec3 v = Vec3(0.f, 0.f, 1.f).rotate_about_xyz(this->rot);
    return v;
}

Vec3 Camera::right() const
{
    Vec3 v = Vec3(1.f, 0.f, 0.f).rotate_about_xyz(this->rot);
    return v;
}

Vec3 Camera::up() const
{
    Vec3 v = Vec3(0.f, 1.f, 0.f).rotate_about_xyz(this->rot);
    return v;
}
