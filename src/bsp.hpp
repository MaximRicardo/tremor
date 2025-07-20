#pragma once

#include "aabb.hpp"
#include "plane.hpp"
#include "triangle.hpp"
#include <memory>
#include <span>
#include <variant>
#include <vector>

class BSP {

    struct LeafInfo {
        // is the current node in empty space or solid space?
        bool empty;
        // triangles lying on the edge of the node's hull
        std::vector<Triangle> edge_tris;
    };

    struct InNodeInfo {
        Plane plane;
    };

    AABB b_box;

    std::variant<std::unique_ptr<LeafInfo>, std::unique_ptr<InNodeInfo>> info;

    bool has_innode_info() const;
    bool has_leaf_info() const;

    const InNodeInfo &innode_info() const;
    InNodeInfo &innode_info();
    const LeafInfo &leaf_info() const;
    LeafInfo &leaf_info();

    void alloc_leaf_nodes();
    void init_leaf_node();
    void create_leaf_nodes(const AABB &cur_box);
    void create_leaf_nodes(std::span<const Triangle> tris);

    // doesn't physically put triangles into the tree, instead uses the provided
    // triangles' planes to create the structure of the tree
    void insert_tris_behind(const Triangle &tri);
    void insert_tris_in_front(const Triangle &tri);
    void insert(const Triangle &tri);
    void insert(std::span<const Triangle> tris);

    // insertion of triangles after the structure of the tree has been finalized
    // and the final LeafInfo::edge_tris can be found
    void leaf_insert_tris_behind(const Triangle &tri);
    void leaf_insert_tris_in_front(const Triangle &tri);
    void leaf_insert(const Triangle &tri);
    void leaf_insert(std::span<const Triangle> tris);

    explicit BSP(BSP *parent);
    BSP(const Plane &plane, BSP *parent);

public:
    // these nodes contain the child tris behind and in front of this
    std::unique_ptr<BSP> behind = nullptr;
    std::unique_ptr<BSP> in_front = nullptr;
    BSP *parent = nullptr;

    explicit BSP(std::span<const Triangle> tris);

    void render(std::span<Color> frame, std::span<float> depth_buffer,
                const Camera &cam, std::span<const Texture> texs) const;
    size_t n_triangles() const;
    size_t max_depth() const;
    bool is_leaf() const;
    // get the leaf node a point is in
    const BSP &get_point_node(const Vec3 &point) const;
    BSP &get_point_node(const Vec3 &point);
    bool point_in_solid(const Vec3 &point) const;
};
