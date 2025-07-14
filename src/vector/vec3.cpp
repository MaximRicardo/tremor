#include "vec3.hpp"
#include <cmath>

Vec3::Vec3() {}
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

Vec3 Vec3::operator-() const
{
    return {-this->x, -this->y, -this->z};
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

Vec3 Vec3::rotate_about_x(Angle amount) const
{
    Vec3 v;
    v.x = this->x;
    v.y = this->y * std::cos(amount.get()) - this->z * std::sin(amount.get());
    v.z = this->y * std::sin(amount.get()) + this->z * std::cos(amount.get());
    return v;
}

Vec3 Vec3::rotate_about_y(Angle amount) const
{
    Vec3 v;
    v.x = this->x * std::cos(amount.get()) - this->z * std::sin(amount.get());
    v.y = this->y;
    v.z = this->x * std::sin(amount.get()) + this->z * std::cos(amount.get());
    return v;
}

Vec3 Vec3::rotate_about_z(Angle amount) const
{
    Vec3 v;
    v.x = this->x * std::cos(amount.get()) - this->y * std::sin(amount.get());
    v.y = this->x * std::sin(amount.get()) + this->y * std::cos(amount.get());
    v.z = this->z;
    return v;
}

Vec3 Vec3::rotate(const EulerAngle &amount) const
{
    Vec3 v = *this;
    v = v.rotate_about_z(amount.z);
    v = v.rotate_about_y(amount.y);
    v = v.rotate_about_x(amount.x);
    return v;
}

Vec3 Vec3::mix(const Vec3 &v, float t) const
{
    return *this + (v - *this) * t;
}

float Vec3::dist(const Vec3 &v) const
{
    Vec3 diff = v - *this;
    return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
}

float Vec3::length() const
{
    return std::sqrt(this->dot(*this));
}

Vec3 Vec3::normalize() const
{
    return *this / this->length();
}
