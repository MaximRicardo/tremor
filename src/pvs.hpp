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
};

} // namespace PVS
