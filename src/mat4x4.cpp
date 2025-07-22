#include "mat4x4.hpp"
#include "vector/vec3.hpp"
#include <array>
#include <cstddef>

Matrix4x4::Matrix4x4(const decltype(elems) &elems) : elems(elems) {}

Matrix4x4 Matrix4x4::identity()
{
    return Matrix4x4(decltype(elems){
        std::array{1.f, 0.f, 0.f, 0.f}, std::array{0.f, 1.f, 0.f, 0.f},
        std::array{0.f, 0.f, 1.f, 0.f}, std::array{0.f, 0.f, 0.f, 1.f}});
}

Matrix4x4 Matrix4x4::translate(const Vec3 &v)
{
    return Matrix4x4(decltype(elems){
        std::array{1.f, 0.f, 0.f, 0.f}, std::array{0.f, 1.f, 0.f, 0.f},
        std::array{0.f, 0.f, 1.f, 0.f}, std::array{v.x, v.y, v.z, 1.f}});
}

Vec4 Matrix4x4::operator*(const Vec4 &v) const
{
    std::array<float, 4> components;
    for (int i = 0; i < 4; ++i) {
        components[i] = this->elems[0][i] * v.x + this->elems[1][i] * v.y +
                        this->elems[2][i] * v.z + this->elems[3][i] * v.w;
    }

    return {components[0], components[1], components[2], components[3]};
}

Matrix4x4 Matrix4x4::operator*(float x) const
{
    Matrix4x4 result(*this);

    for (size_t i = 0; i < this->elems.size(); ++i) {
        for (size_t j = 0; j < this->elems[i].size(); ++j) {
            result.elems[i][j] *= x;
        }
    }

    return result;
}

Matrix4x4 &Matrix4x4::operator*=(float x)
{
    for (size_t i = 0; i < this->elems.size(); ++i) {
        for (size_t j = 0; j < this->elems[i].size(); ++j) {
            this->elems[i][j] *= x;
        }
    }

    return *this;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &other) const
{
    Matrix4x4 result;

    for (size_t i = 0; i < this->elems.size(); ++i) {
        for (size_t j = 0; j < this->elems[i].size(); ++j) {
            float val = 0.f;
            for (int k = 0; k < 4; ++k) {
                val += this->elems[k][j] * other.elems[i][k];
            }

            result.elems[i][j] = val;
        }
    }

    return result;
}

Matrix4x4 &Matrix4x4::operator*=(const Matrix4x4 &other)
{
    // might wanna optimize this at some point
    return *this = *this * other;
}
