#pragma once

#include "aabb.hpp"
#include "constants.hpp"
#include "polygon.hpp"
#include "ssize.hpp"
#include "triangle.hpp"
#include "vector/vec3.hpp"
#include <vector>

// might not actually need to be convex, but lets just say it does to be safe.
class ConvexShape {

    void remove_empty_polys();

public:
    // each poly points outwards. not placed in any order
    std::vector<Polygon> polys;

    explicit ConvexShape(std::span<const Polygon> polys);

    static ConvexShape box(Vec3 scale);

    void clip(const Plane &plane);
    std::vector<Vec3> get_intersections(const Plane &plane) const;
    std::vector<Triangle> get_triangles() const;
    AABB get_aabb() const;
    bool contains(const Vec3 &p, float epsilon = Consts::epsilon) const;
    Vec3 get_center() const;
};

class BrushShape {

    void remove_empty_polys();

public:
    // each poly points outwards. not placed in any order
    std::vector<RenderPolygon> polys;

    explicit BrushShape(std::span<const RenderPolygon> polys);

    // new_tex_idx is the texture index of the newly created polygon caused by
    // the clip, if there is one. if it's equal to -1, the tex idx of the
    // first polygon is used
    void clip(const Plane &plane, isize_t new_tex_idx = -1);
    std::vector<TexVert> get_intersections(const Plane &plane) const;
    std::vector<Triangle> get_triangles() const;
    AABB get_aabb() const;
    bool contains(const Vec3 &p, float epsilon = Consts::epsilon) const;
    Vec3 get_center() const;
};
