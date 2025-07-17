#pragma once

#include "aabb.hpp"
#include "plane.hpp"
#include <array>

class Frustum {

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

    Vec3 forward_vec() const;
    Vec3 right_vec() const;
    Vec3 up_vec() const;

    bool contains(const Vec3 &p) const;
    bool b_box_maybe_inside(const AABB &box) const;
};
