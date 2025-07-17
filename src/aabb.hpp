#pragma once

#include "plane.hpp"
#include "vector/vec3.hpp"
#include <array>
#include <vector>

class AABB {

    std::vector<Vec3> clipped_vertices(const Plane &plane) const;

public:
    Vec3 min, max;

    AABB();
    AABB(Vec3 min, Vec3 max);

    bool is_point_inside(const Vec3 &p) const;
    void clip(const Plane &plane);
    bool intersects(const Plane &plane) const;
    std::array<Vec3, 8> get_vertices() const;
};
