#pragma once

#include "aabb.hpp"
#include "camera.hpp"
#include "frame.hpp"
#include "mat4x4.hpp"
#include "plane.hpp"
#include "vector/vec2.hpp"
#include "vector/vec3.hpp"
#include "vertex.hpp"
#include <span>
#include <vector>

class RenderPolygon;

// is assumed to be counter clockwise
class Polygon {

public:
    std::vector<Vec3> vs;

    // can not and does not sort vs ccw
    explicit Polygon(std::span<const Vec3> vs);
    // automatically sorts vs ccw
    Polygon(std::span<const Vec3> vs, const Vec3 &normal);
    explicit Polygon(const RenderPolygon &other);

    Vec3 get_center() const;
    Plane get_plane() const;
    void sort_vs_ccw(const Vec3 &intended_normal);
    // clips to the part that is in front of the plane
    void clip(const Plane &plane);
    std::vector<Vec3> get_intersections(const Plane &plane) const;
    std::vector<Triangle> get_triangles() const;
    AABB get_aabb() const;
    bool empty() const;
    bool invalid() const; // number of vertices is less than 3
};

class RenderPolygon {

    std::vector<Vec3> get_cam_space_vs(const Matrix4x4 &transform,
                                       const Camera &cam) const;
    std::vector<Vec2i> get_screen_vs(const Matrix4x4 &transform,
                                     const Camera &cam) const;

public:
    std::vector<TexVert> vs;
    size_t tex_idx;

    // can not and does not sort vs ccw
    RenderPolygon(std::span<const Vec3> vs, std::span<const Vec2> vts,
                  size_t tex_idx);
    // automatically sorts vs ccw
    RenderPolygon(std::span<const Vec3> vs, std::span<const Vec2> vts,
                  size_t tex_idx, const Vec3 &normal);

    Vec3 get_center() const;
    Plane get_plane() const;
    void sort_vs_ccw(const Vec3 &intended_normal);
    void clip(const Plane &plane);
    void render(const Matrix4x4 &transform, Frame &frame, const Camera &cam,
                std::span<const Texture> texs) const;
    void apply_transform(const Matrix4x4 &mat);
    std::vector<Triangle> get_triangles() const;
    bool empty() const;
    bool invalid() const; // number of vertices is less than 3
    float get_area() const;
};
