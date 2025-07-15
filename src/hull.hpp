#pragma once

#include "plane.hpp"
#include "polygon.hpp"
#include <vector>

// a hull defined by a group of polygons with a counter-clockwise winding order
// when viewed from outside the hull.
// used for collision detection
class ConvexHull {

    void remove_empty_polys();

public:
    std::vector<Polygon> polys;

    // intializes the hull to a box centered at the origin with a width and
    // height of 20000 units
    ConvexHull();

    void clip(const Plane &plane);
    // the points at which the hull intersects the plane
    std::vector<Vec3> plane_intersections(const Plane &plane) const;

    Vec3 get_center() const;
    // returns true if every polygon has the correct winding order
    bool verify_winding_order() const;

    bool is_point_inside(const Vec3 &p) const;
};
