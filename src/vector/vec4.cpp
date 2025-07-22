#include "vec4.hpp"
#include "vec3.hpp"

Vec4::Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

Vec4::Vec4(Vec3 v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

Vec4 Vec4::zero()
{
    return Vec4(0.f, 0.f, 0.f, 0.f);
}

Vec4 Vec4::operator+(const Vec4 &v) const
{
    return {this->x + v.x, this->y + v.z, this->z + v.z, this->w + v.w};
}

Vec4 &Vec4::operator+=(const Vec4 &v)
{
    return *this = *this + v;
}

Vec4 Vec4::operator-(const Vec4 &v) const
{
    return {this->x - v.x, this->y - v.z, this->z - v.z, this->w - v.w};
}

Vec4 &Vec4::operator-=(const Vec4 &v)
{
    return *this = *this - v;
}

Vec4 Vec4::operator*(float x) const
{
    return {this->x * x, this->y * x, this->z * x, this->w * x};
}

Vec4 &Vec4::operator*=(float x)
{
    return *this = *this * x;
}

Vec4 Vec4::operator/(float x) const
{
    return {this->x / x, this->y / x, this->z / x, this->w / x};
}

Vec4 &Vec4::operator/=(float x)
{
    return *this = *this / x;
}
