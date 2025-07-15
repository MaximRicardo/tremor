#pragma once

#include <ostream>
class Vec2i;

class Vec2 {

public:
    float x, y;

    Vec2();
    Vec2(float x, float y);
    Vec2(const Vec2i &v);

    Vec2 operator+(const Vec2 &v) const;
    Vec2 operator+=(const Vec2 &v);
    Vec2 operator-(const Vec2 &v) const;
    Vec2 operator-=(const Vec2 &v);
    Vec2 operator*(float x) const;
    Vec2 operator*=(float x);
    Vec2 operator/(float x) const;
    Vec2 operator/=(float x);

    float dot(const Vec2 &v) const;
    Vec2 mix(const Vec2 &v, float t) const;
};

std::ostream &operator<<(std::ostream &os, const Vec2 &v);

class Vec2i {

public:
    int x, y;

    Vec2i();
    Vec2i(int x, int y);
    Vec2i(const Vec2 &v);

    Vec2i operator+(const Vec2i &v) const;
    Vec2i operator+=(const Vec2i &v);
    Vec2i operator-(const Vec2i &v) const;
    Vec2i operator-=(const Vec2i &v);
    Vec2i operator*(int x) const;
    Vec2i operator*=(int x);
    Vec2i operator/(int x) const;
    Vec2i operator/=(int x);
};

std::ostream &operator<<(std::ostream &os, const Vec2i &v);
