#include "mat2x2.hpp"
#include <array>

Matrix2x2::Matrix2x2(const std::array<std::array<float, 2>, 2> &elems)
    : elems(elems)
{}

Vec2 Matrix2x2::operator*(const Vec2 &v) const
{
    std::array<float, 2> components;
    for (int i = 0; i < 2; ++i) {
        components[i] = this->elems[0][i] * v.x + this->elems[1][i] * v.y;
    }

    return {components[0], components[1]};
}

Matrix2x2 Matrix2x2::operator*(float x) const
{
    Matrix2x2 result(*this);

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            result.elems[i][j] *= x;
        }
    }

    return result;
}

float Matrix2x2::determinant() const
{
    float a = this->elems[0][0];
    float b = this->elems[1][0];
    float c = this->elems[0][1];
    float d = this->elems[1][1];

    return a * d - b * c;
}
