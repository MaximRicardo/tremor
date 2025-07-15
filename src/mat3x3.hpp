#pragma once

#include "vector/vec3.hpp"
#include <array>

// elements are stored in column major order
class Matrix3x3 {

public:
    std::array<std::array<float, 3>, 3> elems;

    explicit Matrix3x3(const std::array<std::array<float, 3>, 3> &elems);

    Vec3 operator*(const Vec3 &v) const;
    Matrix3x3 operator*(float x) const;

    float determinant() const;
    float minor(int column, int row) const;
    Matrix3x3 cofactor() const;
    Matrix3x3 transpose() const;
    Matrix3x3 inverse() const;
};
