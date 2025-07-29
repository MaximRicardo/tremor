#include "fmath.hpp"
#include <cstdlib>

bool FMath::approx_eq(float a, float b, float epsilon)
{
    float diff = a - b;
    return std::abs(diff) < epsilon;
}
