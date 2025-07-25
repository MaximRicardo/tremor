#pragma once

#include "vector/vec2.hpp"
#include "vector/vec3.hpp"

// a textured vertex
class TexVert {

public:
    Vec3 v;
    Vec2 vt;

    TexVert() = default;
    TexVert(const Vec3 &v, const Vec2 &vt);
};
