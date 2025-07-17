#pragma once

#include "aabb.hpp"
#include "plane.hpp"
#include <array>

class Frustum {

    bool b_box_has_vertex_inside(const AABB &box) const;

public:
    static constexpr size_t near_face = 0;
    static constexpr size_t far_face = 1;
    static constexpr size_t left_face = 2;
    static constexpr size_t right_face = 3;
    static constexpr size_t top_face = 4;
    static constexpr size_t bottom_face = 5;
    static constexpr size_t n_faces = 6;

    // a set of planes making up the frustum. every plane points outwards
    std::array<Plane, n_faces> planes;

    explicit Frustum(std::span<const Plane, n_faces> planes);

    bool b_box_intersects(const AABB &box) const;

    bool point_inside(const Vec3 &p) const;
    // also returns true if the box is entirely inside
    bool b_box_partially_inside(const AABB &box) const;
};
