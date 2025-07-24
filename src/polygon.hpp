#pragma once

#include "aabb.hpp"
#include "plane.hpp"
#include "vector/vec3.hpp"
#include <span>
#include <vector>

// is assumed to be counter clockwise
class Polygon {

public:
    std::vector<Vec3> vs;

    // can not and does not sort vs ccw
    Polygon(std::span<const Vec3> vs);
    // automatically sorts vs ccw
    Polygon(std::span<const Vec3> vs, const Vec3 &normal);

    Vec3 get_center() const;
    Plane get_plane() const;
    void sort_vs_ccw(Vec3 intended_normal);
    // clips to the part that is in front of the plane
    void clip(const Plane &plane);
    std::vector<Vec3> get_intersections(const Plane &plane) const;
    std::vector<Triangle> get_triangles() const;
    AABB get_aabb() const;
};
