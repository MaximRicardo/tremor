#pragma once

#include "hull.hpp"
#include "plane.hpp"
#include "triangle.hpp"
#include <memory>
#include <span>

class BSP {

    void insert_tris_behind(const Triangle &tri);
    void insert_tris_in_front(const Triangle &tri);

    struct LeafInfo {
        // is the current node in empty space or solid space?
        bool empty;
        ConvexHull hull;
    };

    struct InNodeInfo {
        Triangle tri;
        Plane plane;
    };

    std::unique_ptr<LeafInfo> leaf_info;
    std::unique_ptr<InNodeInfo> innode_info;

    void alloc_leaf_nodes();
    void create_leaf_nodes(const ConvexHull &hull);

    explicit BSP(BSP *parent);

public:
    // these nodes contain the child tris behind and in front of this
    std::unique_ptr<BSP> behind = nullptr;
    std::unique_ptr<BSP> in_front = nullptr;
    BSP *parent = nullptr;

    explicit BSP(Triangle tri, BSP *parent = nullptr);
    explicit BSP(std::span<const Triangle> tris, BSP *parent = nullptr);

    // MUST BE CALLED FROM THE ROOT NODE OF THE TREE!
    // MUST BE CALLED AFTER YOU ARE DONE INSERTING TRIANGLES!
    void create_leaf_nodes();

    void insert(const Triangle &tri);
    void insert(const std::span<Triangle> &tris);
    void render(std::span<Color> frame, std::span<float> depth_buffer,
                const Camera &cam, std::span<const Texture> texs) const;
    size_t n_triangles() const;
    bool is_leaf() const;
    // get the leaf node a point is in
    const BSP *get_point_node(const Vec3 &point) const;
    bool point_in_solid(const Vec3 &point) const;
};
