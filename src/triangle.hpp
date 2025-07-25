#pragma once

#include "frame.hpp"
#include "mat4x4.hpp"
#include "sub_triangle.hpp"
#include "texture.hpp"
#include "vector/vec3.hpp"
#include <array>
#include <span>

class Plane;
class Camera;

class Triangle {

public:
    struct ProjectRet {
        std::array<SubTriangle, 2> sub_tris = {SubTriangle(), SubTriangle()};
        unsigned n_sub_tris;
    };

private:
    // n_triangles              - the number of returned sub triangles.
    //                            can be set to 0, 1 or 2.
    ProjectRet project(const Matrix4x4 &transform, const Camera &cam) const;

public:
    // vertices
    std::array<Vec3, 3> vs;
    // vertex texture coords
    std::array<Vec2, 3> vts;

    size_t tex_idx;

    Triangle();
    Triangle(std::array<Vec3, 3> vs, std::array<Vec2, 3> vts, size_t tex_idx);

    void render(const Matrix4x4 &tranform, Frame &frame, const Camera &cam,
                std::span<const Texture> texs) const;

    Plane get_plane() const;
    Plane get_plane(const Matrix4x4 &transform) const;
    float get_area() const;
    bool is_degenerate() const;
};
