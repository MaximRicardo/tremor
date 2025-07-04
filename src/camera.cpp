#include "camera.hpp"
#include "input/input.hpp"

Camera::Camera(Vec3 pos) : pos(pos) {}

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
}
