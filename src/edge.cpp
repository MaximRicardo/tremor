#include "edge.hpp"
#include <cstdint>

Edge::Edge(const SubTriangle *parent, int32_t x, bool is_start)
    : parent(parent), x(x), is_start(is_start)
{}

bool Edge::is_valid() const
{
    return this->parent != nullptr;
}
