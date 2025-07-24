#pragma once

#include <cstdint>

class Line1Di;

class Line1D {

public:
    float min_x = 0.f, max_x = 0.f;

    Line1D() = default;
    Line1D(float min_x, float max_x);
    Line1D(const Line1Di &other);

    bool partially_contains(const Line1D &other) const;
};

class Line1Di {

public:
    int32_t min_x = 0, max_x = 0;

    Line1Di() = default;
    Line1Di(int32_t min_x, int32_t max_x);
    Line1Di(const Line1D &other);

    bool partially_contains(const Line1Di &other) const;
};
