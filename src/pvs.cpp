#include "pvs.hpp"

PVS::Portal::Portal(const Polygon &shape, BSP *in_front, BSP *behind)
    : shape(shape), in_front(in_front), behind(behind)
{}
