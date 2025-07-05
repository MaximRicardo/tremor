#pragma once

class Vec3 {

public:
    float x, y, z;

    Vec3(float x = 0.f, float y = 0.f, float z = 0.f);

    Vec3 operator+(const Vec3 &v) const;
    Vec3 operator+=(const Vec3 &v);
    Vec3 operator-(const Vec3 &v) const;
    Vec3 operator-=(const Vec3 &v);
    Vec3 operator*(float x) const;
    Vec3 operator*=(float x);
    Vec3 operator/(float x) const;
    Vec3 operator/=(float x);

    float dot(const Vec3 &v) const;
    Vec3 cross(const Vec3 &v) const;
    Vec3 rotate_about_y(float radians) const;
};
