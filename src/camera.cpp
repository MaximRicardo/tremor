#include "camera.hpp"
#include "angle.hpp"
#include "constants.hpp"
#include "fov.hpp"
#include "frustum.hpp"
#include "input/input.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <span>

Camera::Camera(Vec3 pos, Angle yaw, Angle pitch, Angle hfov, float move_speed,
               Angle max_pitch, Angle min_pitch)
    : pos(pos), yaw(yaw), pitch(pitch), max_pitch(max_pitch),
      min_pitch(min_pitch), hfov(hfov), vfov(FOV::horizontal_to_vertical(hfov)),
      move_speed(move_speed)
{}

void Camera::limit_rotation()
{
    this->pitch = std::clamp(this->pitch, this->min_pitch, this->max_pitch);

    this->yaw.set(std::fmod(this->yaw.get(Angle::Type::DEGREES), 360.f),
                  Angle::Type::DEGREES);
}

void Camera::handle_input(float delta_time, Screen &screen)
{
    float mov_dist = delta_time * move_speed;
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

// DO NOT FUCKING TOUCH!
Frustum Camera::get_frustum() const
{
    Vec3 forward = this->look_forward_vec();
    Vec3 left = -this->look_right_vec();
    Vec3 up = this->look_up_vec();

    Vec3 near_center = this->pos + forward * Consts::z_near;
    Vec3 far_center = this->pos + forward * Consts::z_far;

    float near_half_h = std::tan(this->vfov.get() / 2.f) * Consts::z_near;
    float near_half_w = std::tan(this->hfov.get() / 2.f) * Consts::z_near;
    float far_half_h = std::tan(this->vfov.get() / 2.f) * Consts::z_far;
    float far_half_w = std::tan(this->hfov.get() / 2.f) * Consts::z_far;

    Vec3 near_top_left = near_center + up * near_half_h - left * near_half_w;
    Vec3 near_top_right = near_center + up * near_half_h + left * near_half_w;
    Vec3 near_bottom_left = near_center - up * near_half_h - left * near_half_w;
    Vec3 near_bottom_right =
        near_center - up * near_half_h + left * near_half_w;

    Vec3 far_top_left = far_center + up * far_half_h - left * far_half_w;
    Vec3 far_top_right = far_center + up * far_half_h + left * far_half_w;
    Vec3 far_bottom_left = far_center - up * far_half_h - left * far_half_w;
    Vec3 far_bottom_right = far_center - up * far_half_h + left * far_half_w;

    std::array<Plane, Frustum::n_faces> planes;
    Vec3 p0, p1, p2;

    p0 = near_bottom_left;
    p1 = far_bottom_left;
    p2 = far_top_left;
    planes[Frustum::left_face] = Plane(std::array{p0, p1, p2});

    p0 = near_top_right;
    p1 = far_top_right;
    p2 = far_bottom_right;
    planes[Frustum::right_face] = Plane(std::array{p0, p1, p2});

    p0 = near_top_left;
    p1 = far_top_left;
    p2 = far_top_right;
    planes[Frustum::top_face] = Plane(std::array{p0, p1, p2});

    p0 = near_bottom_right;
    p1 = far_bottom_right;
    p2 = far_bottom_left;
    planes[Frustum::bottom_face] = Plane(std::array{p0, p1, p2});

    planes[Frustum::near_face] = Plane(-forward, near_center);
    planes[Frustum::far_face] = Plane(forward, far_center);

    return Frustum(planes);
}
