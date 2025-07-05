#pragma once

class Vec2i;

class Vec2 {

public:
    float x, y;

    Vec2(float x = 0.f, float y = 0.f);
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
};

class Vec2i {

public:
    int x, y;

    Vec2i(int x = 0, int y = 0);
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
