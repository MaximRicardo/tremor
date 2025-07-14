#include "camera.hpp"
#include "angle.hpp"
#include "fov.hpp"
#include "input/input.hpp"

Camera::Camera(Vec3 pos, EulerAngle rot, Angle hfov)
    : pos(pos), rot(rot), hfov(hfov), vfov(FOV::horizontal_to_vertical(hfov))
{}

void Camera::handle_input(float delta_time, Screen &screen)
{
    if (Input::is_key_down(Input::Key::W, screen))
        this->pos.z += delta_time;
    if (Input::is_key_down(Input::Key::S, screen))
        this->pos.z -= delta_time;
    if (Input::is_key_down(Input::Key::A, screen))
        this->pos.x -= delta_time;
    if (Input::is_key_down(Input::Key::D, screen))
        this->pos.x += delta_time;

    if (Input::is_key_down(Input::Key::Q, screen))
        this->pos.y -= delta_time;
    if (Input::is_key_down(Input::Key::E, screen))
        this->pos.y += delta_time;

    if (Input::is_key_down(Input::Key::LEFT, screen))
        this->rot.y -= Angle(delta_time);
    if (Input::is_key_down(Input::Key::RIGHT, screen))
        this->rot.y += Angle(delta_time);
}
