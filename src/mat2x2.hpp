#pragma once

#include "vector/vec2.hpp"
#include <array>

// elements are stored in column major order
class Matrix2x2 {

public:
    std::array<std::array<float, 2>, 2> elems;

    explicit Matrix2x2(const std::array<std::array<float, 2>, 2> &elems);

    Vec2 operator*(const Vec2 &v) const;
    Matrix2x2 operator*(float x) const;

    float determinant() const;
};
