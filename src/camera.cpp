#include "camera.hpp"
#include "angle.hpp"
#include "constants.hpp"
#include "fov.hpp"
#include "frustum.hpp"
#include "input/input.hpp"
#include "resolution.hpp"
#include <algorithm>
#include <cmath>

Camera::Camera(Vec3 pos, Angle yaw, Angle pitch, Angle hfov, Angle max_pitch,
               Angle min_pitch)
    : pos(pos), yaw(yaw), pitch(pitch), max_pitch(max_pitch),
      min_pitch(min_pitch), hfov(hfov), vfov(FOV::horizontal_to_vertical(hfov))
{}

void Camera::limit_rotation()
{
    this->pitch = std::clamp(this->pitch, this->min_pitch, this->max_pitch);

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

Frustum Camera::get_frustum() const
{
    Vec3 forward = this->look_forward_vec();
    Vec3 right = this->look_right_vec();
    Vec3 up = this->look_up_vec();

    float far_half_h = Consts::z_far * std::tan(this->vfov.get() * 0.5f);
    float far_half_w = Consts::z_far * std::tan(this->hfov.get() * 0.5f);
    Vec3 far_center = forward * Consts::z_far;

    std::array<Plane, Frustum::n_faces> planes;

    planes[Frustum::near_face] =
        Plane(-forward, -(forward.dot(this->pos) + Consts::z_near));
    planes[Frustum::far_face] =
        Plane(forward, forward.dot(this->pos) + Consts::z_far);

    planes[Frustum::right_face] = Plane(
        ((far_center + right * far_half_w) - this->pos).cross(up), this->pos);
    planes[Frustum::left_face] = Plane(
        (-(far_center - right * far_half_w) - this->pos).cross(up), this->pos);

    planes[Frustum::top_face] = Plane(
        (-(far_center + up * far_half_h) - this->pos).cross(right), this->pos);
    planes[Frustum::bottom_face] = Plane(
        ((far_center - up * far_half_h) - this->pos).cross(right), this->pos);

    return Frustum(planes);
}
