#include "mat3x3.hpp"
#include "mat2x2.hpp"
#include <array>
#include <cassert>
#include <cmath>

Matrix3x3::Matrix3x3(const decltype(elems) &elems) : elems(elems) {}

Vec3 Matrix3x3::operator*(const Vec3 &v) const
{
    std::array<float, 3> components;
    for (int i = 0; i < 3; ++i) {
        components[i] = this->elems[0][i] * v.x + this->elems[1][i] * v.y +
                        this->elems[2][i] * v.z;
    }

    return {components[0], components[1], components[2]};
}

Matrix3x3 Matrix3x3::operator*(float x) const
{
    Matrix3x3 result(*this);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.elems[i][j] *= x;
        }
    }

    return result;
}

float Matrix3x3::determinant() const
{
    float a = this->elems[0][0];
    float b = this->elems[1][0];
    float c = this->elems[2][0];

    float d = this->elems[0][1];
    float e = this->elems[1][1];
    float f = this->elems[2][1];

    float g = this->elems[0][2];
    float h = this->elems[1][2];
    float i = this->elems[2][2];

    return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

float Matrix3x3::minor(int column, int row) const
{
    decltype(Matrix2x2::elems) minor;
    for (int i = 0, minor_i = 0; i < 3; ++i) {
        if (minor_i == column)
            continue;

        for (int j = 0, minor_j = 0; j < 3; ++j) {
            if (minor_j == row)
                continue;

            minor[minor_i][minor_j] = this->elems[i][j];
            ++minor_j;
        }
        ++minor_i;
    }

    return Matrix2x2(minor).determinant();
}

Matrix3x3 Matrix3x3::cofactor() const
{
    decltype(Matrix3x3::elems) cof;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            cof[i][j] = this->minor(i, j) * std::pow(-1, i + j);
        }
    }

    return Matrix3x3(cof);
}

Matrix3x3 Matrix3x3::transpose() const
{
    decltype(Matrix3x3::elems) tpose;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            tpose[j][i] = this->elems[i][j];
        }
    }

    return Matrix3x3(tpose);
}

Matrix3x3 Matrix3x3::inverse() const
{
    float determ = this->determinant();
    assert(determ != 0.f);

    float d = 1.f / determ;

    return this->cofactor().transpose() * d;
}
