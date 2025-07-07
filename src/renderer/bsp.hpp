#pragma once

#include "plane.hpp"
#include "triangle.hpp"
#include <memory>
#include <vector>

class BSP {

    void insert_tris_behind(const Triangle &tri);
    void insert_tris_in_front(const Triangle &tri);

public:
    Triangle node_tri;
    Plane node_plane;

    std::unique_ptr<BSP> lhs = nullptr; // contains the tris behind self
    std::unique_ptr<BSP> rhs = nullptr; // contains the tris in front of self
    BSP *parent = nullptr;

    explicit BSP(Triangle tri, BSP *parent = nullptr);
    explicit BSP(const std::vector<Triangle> &tris, BSP *parent = nullptr);

    void insert(const Triangle &tri);
    void insert(const std::vector<Triangle> &tris);
    void render(Color *frame, float *depth_buffer, const Camera &cam,
                const Texture *texs) const;
    size_t n_triangles() const;
};
