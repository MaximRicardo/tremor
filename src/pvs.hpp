#pragma once

#include "polygon.hpp"

class BSP;

namespace PVS {

class Portal {

public:
    Polygon shape;
    BSP *in_front = nullptr;
    BSP *behind = nullptr;

    Portal(const Polygon &shape, BSP *in_front, BSP *behind);
};

} // namespace PVS
