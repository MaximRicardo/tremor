#pragma once

#include <cstdint>

class SubTriangle;

// used to construct sorted spans for front-to-back rendering
class Edge {

public:
    const SubTriangle *parent = nullptr;
    int32_t x; // y position is implied
    bool is_start;

    Edge() = default;
    Edge(const SubTriangle *parent, int32_t x, bool is_start);

    bool is_valid() const;
};
