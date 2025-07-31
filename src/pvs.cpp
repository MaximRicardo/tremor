#include "pvs.hpp"
#include "bsp.hpp"
#include "camera.hpp"
#include "frame.hpp"
#include "mat4x4.hpp"
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

    bool a_smallest = a.get_shape().get_area() < b.get_shape().get_area();
    const Portal &smallest = a_smallest ? a : b;
    Portal &largest = a_smallest ? b : a;

    auto new_shape = smallest.shape;
    new_shape.flip_dir();
    largest.set_shape(new_shape);
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

void PVS::Portal::cache_tris() const
{
    if (!this->tris_cached) {
        this->tris_cached = true;
        this->tris = this->shape.get_triangles();
    }
}

bool PVS::Portal::is_visible(const Matrix4x4 &transform, Frame &frame,
                             const Camera &cam) const
{
    this->cache_tris();

    for (const auto &tri : tris) {
        if (tri.is_visible(transform, frame, cam))
            return true;
    }

    return false;
}

const Polygon &PVS::Portal::get_shape() const
{
    return this->shape;
}

Polygon &PVS::Portal::get_mut_shape()
{
    this->tris_cached = false;
    return this->shape;
}

void PVS::Portal::set_shape(const Polygon &shape)
{
    this->shape = shape;
    this->cache_tris();
}

const std::vector<Triangle> &PVS::Portal::get_triangles() const
{
    this->cache_tris();
    return this->tris;
}
