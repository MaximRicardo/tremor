#pragma once

#include "color.hpp"
#include "utils/fixed_array.hpp"

class Frame {

public:
    FixedArray<Color> pixels;
    FixedArray<float> depths;

    Frame();

    void clear();
    void update_resolution();
};
