#include "line.hpp"
#include "constants.hpp"
#include <cstdint>

Line1D::Line1D(float min_x, float max_x) : min_x(min_x), max_x(max_x) {}

Line1D::Line1D(const Line1Di &other) : min_x(other.min_x), max_x(other.max_x) {}

bool Line1D::partially_contains(const Line1D &other) const
{
    return this->max_x >= other.min_x - Consts::epsilon &&
           other.max_x >= this->min_x - Consts::epsilon;
}

Line1Di::Line1Di(int32_t min_x, int32_t max_x) : min_x(min_x), max_x(max_x) {}

Line1Di::Line1Di(const Line1D &other) : min_x(other.min_x), max_x(other.max_x)
{}

bool Line1Di::partially_contains(const Line1Di &other) const
{
    return this->max_x >= other.min_x && other.max_x >= this->min_x;
}
