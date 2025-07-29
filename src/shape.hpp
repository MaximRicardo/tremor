#pragma once

#include "aabb.hpp"
#include "constants.hpp"
#include "polygon.hpp"
#include "triangle.hpp"
#include "vector/vec3.hpp"
#include <vector>

// might not actually need to be convex, but lets just say it does to be safe.
class ConvexShape {

    void remove_empty_polys();

public:
    // each poly points outwards. not placed in any order
    std::vector<Polygon> polys;

    explicit ConvexShape(std::span<const Polygon> polys);

    static ConvexShape box(Vec3 scale);

    void clip(const Plane &plane);
    std::vector<Vec3> get_intersections(const Plane &plane) const;
    std::vector<Triangle> get_triangles() const;
    AABB get_aabb() const;
    bool contains(const Vec3 &p, float epsilon = Consts::epsilon) const;
    Vec3 get_center() const;
};
