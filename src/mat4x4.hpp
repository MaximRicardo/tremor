#pragma once

#include "vector/vec3.hpp"
#include "vector/vec4.hpp"
#include <array>

// i fucking hate matrices now

class Matrix4x4 {

    Matrix4x4() = default;

public:
    std::array<std::array<float, 4>, 4> elems;

    explicit Matrix4x4(const decltype(elems) &elems);

    static Matrix4x4 identity();
    static Matrix4x4 translate(const Vec3 &v);

    Vec4 operator*(const Vec4 &v) const;
    Matrix4x4 operator*(float x) const;
    Matrix4x4 &operator*=(float x);
    Matrix4x4 operator*(const Matrix4x4 &other) const;
    Matrix4x4 &operator*=(const Matrix4x4 &other);

    /*
    float minor(size_t column, size_t row) const;
    float cofactor(size_t column, size_t row) const;
    Matrix4x4 transpose() const;
    float determinant() const;
    Matrix4x4 adjugate() const;
    Matrix4x4 inverse() const;
    */
};
