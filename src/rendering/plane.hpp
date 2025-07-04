#pragma once

#include "../vector/vec3.hpp"
#include <array>
#include <tuple>

class Plane {

public:
    // should always have a length of 1
    Vec3 normal;
    float d;

    Plane(Vec3 normal = Vec3(0.f, 0.f, 0.f), float d = 0.f);

    std::tuple<Vec3, float> line_intersect_point(const Vec3 &start,
                                                 const Vec3 &end) const;

    // returns the part of the triangle vs that is in front of this.
    // second return value is for the number of triangles vs had to be split
    // into to represent the resulting shape. can be 0, 1, or 2
    // vectors aren't used cuz from what i can tell the compiler doesn't
    // optimize vectors into arrays and this'll eventually be performance
    // critical
    std::tuple<std::array<std::array<Vec3, 3>, 2>, unsigned>
    clip(const std::array<Vec3, 3> &vs) const;
};
