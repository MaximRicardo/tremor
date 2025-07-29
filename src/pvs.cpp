#include "pvs.hpp"

PVS::Portal::Portal(const Polygon &shape, BSP *behind, BSP *in_front)
    : shape(shape), behind(behind), in_front(in_front)
{}
