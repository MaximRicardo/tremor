#pragma once

#include "plane.hpp"
#include "vector/vec3.hpp"
#include <array>
#include <vector>

class AABB {

    std::vector<Vec3> clipped_vertices(const Plane &plane) const;

public:
    Vec3 min, max;

    AABB() = default;
    AABB(Vec3 min, Vec3 max);

    // creates an AABB spanning the maximum allowed map size
    static AABB map_box();

    bool contains(const Vec3 &p) const;
    bool contains(const AABB &box) const;
    bool partially_contains(const AABB &box) const;
    void clip(const Plane &plane);
    bool intersects(const Plane &plane) const;
    std::vector<Vec3> intersection_points(const Plane &plane) const;
    std::array<Vec3, 8> get_vertices() const;
    void merge(const Vec3 &p);
    void merge(const AABB &other);
    Vec3 get_center() const;
};
