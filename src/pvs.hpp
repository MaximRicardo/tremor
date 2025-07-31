#pragma once

#include "polygon.hpp"
#include <optional>

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

    // quake-style portal visibility algorithm
    // returns the clipped portal if it is visible
    static std::optional<Polygon>
    is_visible(const Polygon &start, const Portal &end, const Polygon &pass);
};

} // namespace PVS
