#include "vec3.hpp"

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
