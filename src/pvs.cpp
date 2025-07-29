#include "pvs.hpp"
#include "bsp.hpp"
#include <cassert>

PVS::Portal::Portal(const Polygon &shape, BSP *behind, BSP *in_front)
    : shape(shape), behind(behind), in_front(in_front)
{}

void PVS::Portal::merge(Portal &a, Portal &b)
{
    assert(a.behind->is_leaf());
    assert(b.behind->is_leaf());

    a.in_front = b.behind;
    b.in_front = a.behind;

    bool a_smallest = a.shape.get_area() < b.shape.get_area();
    const Portal &smallest = a_smallest ? a : b;
    Portal &largest = a_smallest ? b : a;

    largest.shape = smallest.shape;
    largest.shape.flip_dir();
}

void PVS::Portal::merge_with(const Portal &other)
{
    assert(this->behind->is_leaf());
    assert(other.behind->is_leaf());

    this->in_front = other.behind;

    bool other_smallest = other.shape.get_area() < this->shape.get_area();
    if (other_smallest) {
        this->shape = other.shape;
        this->shape.flip_dir();
    }
}
