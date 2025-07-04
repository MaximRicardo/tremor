#include "vec2.hpp"

Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2 Vec2::operator+(const Vec2 &v) const
{
    return {this->x + v.x, this->y + v.y};
}

Vec2 Vec2::operator+=(const Vec2 &v)
{
    *this = *this + v;
    return *this;
}

Vec2 Vec2::operator-(const Vec2 &v) const
{
    return {this->x - v.x, this->y - v.y};
}

Vec2 Vec2::operator-=(const Vec2 &v)
{
    *this = *this - v;
    return *this;
}

Vec2 Vec2::operator*(float x) const
{
    return {this->x * x, this->y * x};
}

Vec2 Vec2::operator*=(float x)
{
    *this = *this * x;
    return *this;
}

Vec2 Vec2::operator/(float x) const
{
    return {this->x / x, this->y / x};
}

Vec2 Vec2::operator/=(float x)
{
    *this = *this / x;
    return *this;
}

float Vec2::dot(const Vec2 &v) const
{
    return this->x * v.x + this->y * v.y;
}
