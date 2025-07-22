#pragma once

class Vec3;

class Vec4 {

public:
    float x, y, z, w;

    Vec4() = default;
    Vec4(float x, float y, float z, float w);
    Vec4(Vec3 v, float w);

    static Vec4 zero();

    Vec4 operator+(const Vec4 &v) const;
    Vec4 &operator+=(const Vec4 &v);
    Vec4 operator-(const Vec4 &v) const;
    Vec4 &operator-=(const Vec4 &v);
    Vec4 operator*(float x) const;
    Vec4 &operator*=(float x);
    Vec4 operator/(float x) const;
    Vec4 &operator/=(float x);
};
