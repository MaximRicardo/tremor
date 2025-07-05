#include "vec3.hpp"
#include <cmath>

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

Vec3 Vec3::operator+(const Vec3 &v) const
{
    return {this->x + v.x, this->y + v.y, this->z + v.z};
}

Vec3 Vec3::operator+=(const Vec3 &v)
{
    *this = *this + v;
    return *this;
}

Vec3 Vec3::operator-(const Vec3 &v) const
{
    return {this->x - v.x, this->y - v.y, this->z - v.z};
}

Vec3 Vec3::operator-=(const Vec3 &v)
{
    *this = *this - v;
    return *this;
}

Vec3 Vec3::operator*(float x) const
{
    return {this->x * x, this->y * x, this->z * x};
}

Vec3 Vec3::operator*=(float x)
{
    *this = *this * x;
    return *this;
}

Vec3 Vec3::operator/(float x) const
{
    return {this->x / x, this->y / x, this->z / x};
}

Vec3 Vec3::operator/=(float x)
{
    *this = *this / x;
    return *this;
}

float Vec3::dot(const Vec3 &v) const
{
    return this->x * v.x + this->y * v.y + this->z * v.z;
}

Vec3 Vec3::cross(const Vec3 &v) const
{
    return {this->y * v.z - this->z * v.y, this->z * v.x - this->x * v.z,
            this->x * v.y - this->y * v.x};
}

Vec3 Vec3::rotate_about_y(float radians) const
{
    Vec3 w;
    w.x = this->x * std::cos(radians) - this->z * std::sin(radians);
    w.y = this->y;
    w.z = this->x * std::sin(radians) + this->z * std::cos(radians);
    return w;
}
