#pragma once

#include "plane.hpp"
#include "triangle.hpp"
#include <memory>
#include <span>

class BSP {

    void insert_tris_behind(const Triangle &tri);
    void insert_tris_in_front(const Triangle &tri);

public:
    Triangle node_tri;
    Plane node_plane;

    // these nodes contain the child tris behind and in front of this
    std::unique_ptr<BSP> behind = nullptr;
    std::unique_ptr<BSP> in_front = nullptr;
    BSP *parent = nullptr;

    explicit BSP(Triangle tri, BSP *parent = nullptr);
    explicit BSP(std::span<const Triangle> tris, BSP *parent = nullptr);

    void insert(const Triangle &tri);
    void insert(const std::span<Triangle> &tris);
    void render(std::span<Color> frame, std::span<float> depth_buffer,
                const Camera &cam, std::span<const Texture> texs) const;
    size_t n_triangles() const;
};
