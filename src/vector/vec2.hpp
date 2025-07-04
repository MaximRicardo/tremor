#pragma once

class Vec2 {

public:
    float x, y;

    Vec2(float x = 0.f, float y = 0.f);

    Vec2 operator+(const Vec2 &v) const;
    Vec2 operator+=(const Vec2 &v);
    Vec2 operator-(const Vec2 &v) const;
    Vec2 operator-=(const Vec2 &v);
    Vec2 operator*(float x) const;
    Vec2 operator*=(float x);
    Vec2 operator/(float x) const;
    Vec2 operator/=(float x);

    float dot(const Vec2 &v) const;
};
