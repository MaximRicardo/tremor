#pragma once

#include "color.hpp"
#include "edge.hpp"
#include "utils/fixed_array.hpp"
#include <list>

class Scanline {

public:
    std::list<Edge> edges;

    void clear();
    void add_edge(const Edge &edge);
};

class Frame {

public:
    // one elem for each pixel
    FixedArray<Color> pixels;
    FixedArray<float> depths;

    // one elem for each row
    FixedArray<Scanline> scans;

    Frame();

    void clear();
    void update_resolution();
};
