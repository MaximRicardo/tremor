#pragma once

#include "../angle.hpp"
#include <ostream>

class Vec3 {

public:
    float x, y, z;

    Vec3();
    Vec3(float x, float y, float z);

    Vec3 operator+(const Vec3 &v) const;
    Vec3 operator+=(const Vec3 &v);
    Vec3 operator-(const Vec3 &v) const;
    Vec3 operator-=(const Vec3 &v);
    Vec3 operator*(float x) const;
    Vec3 operator*=(float x);
    Vec3 operator/(float x) const;
    Vec3 operator/=(float x);
    Vec3 operator-() const;

    float dot(const Vec3 &v) const;
    Vec3 cross(const Vec3 &v) const;
    Vec3 rotate_about_x(Angle amount) const;
    Vec3 rotate_about_y(Angle amount) const;
    Vec3 rotate_about_z(Angle amount) const;
    // rotates about the z axis, then the y axis, then the x axis
    Vec3 rotate(const EulerAngle &amount) const;
    Vec3 rotate(Angle amount, Vec3 axis) const;
    Vec3 mix(const Vec3 &v, float t) const;
    float dist(const Vec3 &v) const;
    float length() const;
    Vec3 normalize() const;
    Angle angle_between(const Vec3 &v) const;
    // assumes the normal is normalized
    Vec3 project(Vec3 normal) const;
};

std::ostream &operator<<(std::ostream &os, const Vec3 &v);
