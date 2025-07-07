#pragma once

#include "../vector/vec3.hpp"
#include "triangle.hpp"
#include <array>
#include <tuple>

class Plane {

public:
    // should always have a length of 1
    Vec3 normal;
    float d;

    Plane(Vec3 normal, float d);

    std::tuple<Vec3, float> line_intersect_point(const Vec3 &start,
                                                 const Vec3 &end) const;

    struct ClipTriangleRet {
        std::array<Triangle, 2> tris = {Triangle({}, {}), Triangle({}, {})};
        unsigned n_tris;
    };

    // returns the part of the triangle vs that is in front of this.
    ClipTriangleRet clip(const Triangle &tri) const;
    bool is_coplanar(const Plane &plane) const;
};
