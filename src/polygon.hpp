#pragma once

#include "plane.hpp"
#include "vector/vec3.hpp"
#include <span>
#include <vector>

// a flat, convex 3D polygon with a counter-clockwise winding order
class Polygon {

public:
    std::vector<Vec3> vs;

    Polygon(std::span<const Vec3> vs);

    Plane get_plane() const;
    void clip(const Plane &plane);
    // the points at which the polygon intersects the plane
    std::vector<Vec3> plane_intersections(const Plane &plane) const;
};
