#pragma once

#include "plane.hpp"
#include "vector/vec3.hpp"
#include <span>
#include <vector>

// is assumed to be counter clockwise
class Polygon {

public:
    std::vector<Vec3> vs;

    Polygon(std::span<const Vec3> vs);

    Vec3 get_center() const;
    Plane get_plane() const;
    void sort_vs_ccw(Vec3 intended_normal);
};
