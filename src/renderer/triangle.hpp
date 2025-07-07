#pragma once

#include "../camera.hpp"
#include "../color.hpp"
#include "../texture.hpp"
#include "../vector/vec3.hpp"
#include "sub_triangle.hpp"
#include <array>

class Plane;

class Triangle {

public:
    struct ProjectRet {
        std::array<SubTriangle, 2> sub_tris = {SubTriangle({}, {}),
                                               SubTriangle({}, {})};
        unsigned n_sub_tris;
    };

private:
    // n_triangles              - the number of returned sub triangles.
    //                            can be set to 0, 1 or 2.
    ProjectRet project(const Camera &cam) const;

public:
    // vertices
    std::array<Vec3, 3> vs;
    // vertex texture coords
    std::array<Vec2, 3> vts;

    Triangle(std::array<Vec3, 3> vs, std::array<Vec2, 3> vts);

    void render(Color *frame, float *depth_buffer, const Camera &cam,
                const Texture *texs) const;

    Plane get_plane() const;
};
