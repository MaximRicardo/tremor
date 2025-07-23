#include "mat4x4.hpp"
#include "vector/vec3.hpp"
#include <array>
#include <cmath>
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

Matrix4x4 Matrix4x4::scale(const Vec3 &v)
{
    return Matrix4x4(decltype(elems){
        std::array{v.x, 0.f, 0.f, 0.f}, std::array{0.f, v.y, 0.f, 0.f},
        std::array{0.f, 0.f, v.z, 0.f}, std::array{0.f, 0.f, 0.f, 1.f}});
}

Matrix4x4 Matrix4x4::rotation_x(const Angle &angle)
{
    float c = std::cos(angle.get());
    float s = std::sin(angle.get());

    return Matrix4x4(decltype(elems){
        std::array{1.f, 0.f, 0.f, 0.f}, std::array{0.f, c, s, 0.f},
        std::array{0.f, -s, c, 0.f}, std::array{0.f, 0.f, 0.f, 1.f}});
}

Matrix4x4 Matrix4x4::rotation_y(const Angle &angle)
{
    float c = std::cos(angle.get());
    float s = std::sin(angle.get());

    return Matrix4x4(decltype(elems){
        std::array{c, 0.f, -s, 0.f}, std::array{0.f, 1.f, 0.f, 0.f},
        std::array{s, 0.f, c, 0.f}, std::array{0.f, 0.f, 0.f, 1.f}});
}

Matrix4x4 Matrix4x4::rotation_z(const Angle &angle)
{
    float c = std::cos(angle.get());
    float s = std::sin(angle.get());

    return Matrix4x4(decltype(elems){
        std::array{c, s, 0.f, 0.f}, std::array{-s, c, 0.f, 0.f},
        std::array{0.f, 0.f, 1.f, 0.f}, std::array{0.f, 0.f, 0.f, 1.f}});
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

// THE BELOW FUNCTIONS DO NOT WORK
#ifdef m_COMMENT
float Matrix4x4::minor(size_t column, size_t row) const
{
    decltype(Matrix3x3::elems) minor;
    for (size_t i = 0, minor_i = 0; i < this->elems.size(); ++i) {
        if (minor_i == column)
            continue;

        for (size_t j = 0, minor_j = 0; j < this->elems[i].size(); ++j) {
            if (minor_j == row)
                continue;

            minor[minor_i][minor_j] = this->elems[i][j];
            ++minor_j;
        }
        ++minor_i;
    }

    return Matrix3x3(minor).determinant();
}

float Matrix4x4::cofactor(size_t column, size_t row) const
{
    size_t sum = column + row;

    if (sum % 2 == 0)
        return this->minor(column, row);
    else
        return -this->minor(column, row);
}

Matrix4x4 Matrix4x4::transpose() const
{
    decltype(elems) tpose;

    for (size_t i = 0; i < this->elems.size(); ++i) {
        for (size_t j = 0; j < this->elems[i].size(); ++j) {
            tpose[j][i] = this->elems[i][j];
        }
    }

    return Matrix4x4(tpose);
}

float Matrix4x4::determinant() const
{
    size_t column = 0;

    float sum = 0.f;
    for (size_t row = 0; row < this->elems[column].size(); ++row) {
        sum += this->elems[column][row] * this->cofactor(column, row);
    }

    return sum;
}

Matrix4x4 Matrix4x4::adjugate() const
{
    decltype(elems) result;

    for (size_t i = 0; i < this->elems.size(); ++i) {
        for (size_t j = 0; j < this->elems[i].size(); ++j) {
            result[j][i] = this->cofactor(i, j);
        }
    }

    return Matrix4x4(result);
}

Matrix4x4 Matrix4x4::inverse() const
{
    std::cout << "determinant = " << this->determinant() << "\n";
    return this->adjugate() * 0.5f * this->determinant();
}
#endif
