#pragma once

#include "polygon.hpp"
#include <vector>

class BSP;

namespace PVS {

class Portal {

    mutable bool tris_cached = false;
    mutable std::vector<Triangle> tris;
    Polygon shape;

    void cache_tris() const;

public:
    BSP *behind = nullptr;
    BSP *in_front = nullptr;

    Portal(const Polygon &shape, BSP *behind, BSP *in_front);

    static void merge(Portal &a, Portal &b);
    void merge_with(const Portal &other);
    bool are_tris_cached() const;
    bool is_visible(const Matrix4x4 &transform, Frame &frame,
                    const Camera &cam) const;
    const Polygon &get_shape() const;
    // NOTE: SLOWER THAN CONST VERSION DUE TO TRIANGLES NEEDING TO BE RE-CACHED!
    Polygon &get_mut_shape();
    void set_shape(const Polygon &shape);
    const std::vector<Triangle> &get_triangles() const;
};

} // namespace PVS
