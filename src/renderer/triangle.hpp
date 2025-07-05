#pragma once

#include "../camera.hpp"
#include "../color.hpp"
#include "../vector/vec3.hpp"
#include "sub_triangle.hpp"
#include <array>

class Triangle {

public:
    struct ProjectRet {
        std::array<SubTriangle, 2> sub_tris;
        unsigned n_sub_tris;
    };

private:
    // n_triangles              - the number of returned sub triangles.
    //                            can be set to 0, 1 or 2.
    ProjectRet project(Camera &cam) const;

public:
    std::array<Vec3, 3> vs;

    Triangle(Vec3 v_0, Vec3 v_1, Vec3 v_2);

    void render(Color *frame, Camera &cam);
};
