#pragma once

#include "constants.hpp"
#include "triangle.hpp"
#include "vector/vec3.hpp"
#include <array>
#include <span>
#include <tuple>

class Plane {

public:
    // should always have a length of 1
    Vec3 normal;
    float d;

    Plane();
    // DOES NOT AUTOMATICALLY NORMALIZE THE NORMAL
    Plane(Vec3 normal, float d);
    // p is a point on the plane
    Plane(Vec3 normal, Vec3 p);
    // creates a plane from a ccw triangle
    explicit Plane(std::span<const Vec3, 3> vs);

    bool does_line_intersect(const Vec3 &start, const Vec3 &end) const;
    // returns the intersection point and it's distance from start, relative to
    // the distance between start and end.
    // ASSUMES THE LINE ACTUALLY INTERSECTS!
    std::tuple<Vec3, float> line_intersect_point(const Vec3 &start,
                                                 const Vec3 &end) const;

    struct ClipTriangleRet {
        std::array<Triangle, 2> tris = {Triangle({}, {}, 0),
                                        Triangle({}, {}, 0)};
        unsigned n_tris;
    };

    // TODO: move this function into the Triangle class instead
    // returns the part of the triangle vs that is in front of this.
    ClipTriangleRet clip(const Triangle &tri) const;

    bool is_coplanar(const Plane &plane, float epsilon = Consts::epsilon) const;
    bool is_point_behind(const Vec3 &p) const;

    Plane flipped() const;
};
