#include "vec2.hpp"
#include "vec3.hpp"
#include <cmath>

Vec2::Vec2() {};

Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2::Vec2(const Vec2i &v) : x(v.x), y(v.y) {}

Vec2::Vec2(const Vec3 &v) : x(v.x), y(v.y) {};

Vec2 Vec2::zero()
{
    return {0.f, 0.f};
}

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

Vec2 Vec2::operator*(const Vec2 &v) const
{
    return {this->x * v.x, this->y * v.y};
}

Vec2 Vec2::operator*=(const Vec2 &v)
{
    *this = *this * v;
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

Vec2 Vec2::operator/(const Vec2 &v) const
{
    return {this->x / v.x, this->y / v.y};
}

Vec2 Vec2::operator/=(const Vec2 &v)
{
    *this = *this / v;
    return *this;
}

float Vec2::dot(const Vec2 &v) const
{
    return this->x * v.x + this->y * v.y;
}

Vec2 Vec2::mix(const Vec2 &v, float t) const
{
    return *this + (v - *this) * t;
}

float Vec2::dist(const Vec2 &v) const
{
    float diff_x = v.x - this->x;
    float diff_y = v.y - this->y;
    return std::sqrt(diff_x * diff_x + diff_y * diff_y);
}

Vec2i::Vec2i() {}
Vec2i::Vec2i(int x, int y) : x(x), y(y) {}
Vec2i::Vec2i(const Vec2 &v) : x(v.x), y(v.y) {}

Vec2i Vec2i::zero()
{
    return {0, 0};
}

Vec2i Vec2i::operator+(const Vec2i &v) const
{
    return {this->x + v.x, this->y + v.y};
}

Vec2i Vec2i::operator+=(const Vec2i &v)
{
    *this = *this + v;
    return *this;
}

Vec2i Vec2i::operator-(const Vec2i &v) const
{
    return {this->x - v.x, this->y - v.y};
}

Vec2i Vec2i::operator-=(const Vec2i &v)
{
    *this = *this - v;
    return *this;
}

Vec2i Vec2i::operator*(int x) const
{
    return {this->x * x, this->y * y};
}

Vec2i Vec2i::operator*=(int x)
{
    *this = *this * x;
    return *this;
}

Vec2i Vec2i::operator*(const Vec2i &v) const
{
    return {this->x * v.x, this->y * v.y};
}

Vec2i Vec2i::operator*=(const Vec2i &v)
{
    *this = *this * v;
    return *this;
}

Vec2i Vec2i::operator/(int x) const
{
    return {this->x / x, this->y / y};
}

Vec2i Vec2i::operator/=(int x)
{
    *this = *this / x;
    return *this;
}

Vec2i Vec2i::operator/(const Vec2i &v) const
{
    return {this->x / v.x, this->y / v.y};
}

Vec2i Vec2i::operator/=(const Vec2i &v)
{
    *this = *this / v;
    return *this;
}

int Vec2i::dist(const Vec2 &v) const
{
    int diff_x = v.x - this->x;
    int diff_y = v.y - this->y;
    return std::sqrt(diff_x * diff_x + diff_y * diff_y);
}

std::ostream &operator<<(std::ostream &os, const Vec2 &v)
{
    os << v.x << ", " << v.y;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Vec2i &v)
{
    os << v.x << ", " << v.y;
    return os;
}
