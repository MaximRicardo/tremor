#pragma once

#include "polygon.hpp"

class BSP;

namespace PVS {

class Portal {

public:
    Polygon shape;
    BSP *behind = nullptr;
    BSP *in_front = nullptr;

    Portal(const Polygon &shape, BSP *behind, BSP *in_front);

    static void merge(Portal &a, Portal &b);
    void merge_with(const Portal &other);
    bool are_tris_cached() const;
};

} // namespace PVS
