#pragma once

#include "frame.hpp"
#include "plane.hpp"
#include "polygon.hpp"
#include "shape.hpp"
#include "triangle.hpp"
#include <cstdint>
#include <memory>
#include <span>
#include <variant>
#include <vector>

class MapEntity;

// a quake-style binary space partitioning tree.
// each innode holds a splitting plane, and each empty leaf node holds a set
// of triangles along the boundaries of the convex sub-space the leaf
// represents, with each triangle pointing inwards towards the center of the
// node's sub-space. each leaf node's set of triangles can be rendered with zero
// overdraw via backface culling.
class BSP {

    struct LeafInfo {
        // is the current node in empty space or solid space?
        bool empty;
        // triangles lying on the edge of the node's hull.
        // WILL BE IMPORTANT FOR PVS! IF YOU DECIDE TO NOT ADD THAT, THIS CAN
        // PROBABLY BE REMOVED!
        std::vector<Triangle *> edge_tris;
    };

    struct InNodeInfo {
        Plane plane;
        // triangles which are coplanar with plane
        std::vector<Triangle> tris;
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
    void init_leaf_node(const std::vector<Triangle *> &tris);
    void create_leaf_nodes(const ConvexShape &cur_hull,
                           std::vector<Triangle *> tris);
    void create_leaf_nodes();

    // doesn't physically put triangles into the tree, instead uses the provided
    // triangles' planes to create the structure of the tree
    void insert_poly_behind(const RenderPolygon &poly);
    void insert_poly_in_front(const RenderPolygon &poly);
    void insert(const RenderPolygon &poly);
    void create_outline(std::span<const RenderPolygon> polys);

    void render_innode_tris(const MapEntity &parent, Frame &frame,
                            const Camera &cam,
                            std::span<const Texture> texs) const;
    void render_leaf_node_tris(const MapEntity &parent, Frame &frame,
                               const Camera &cam,
                               std::span<const Texture> texs) const;

    explicit BSP(BSP *parent);
    // innode constructor
    BSP(const Plane &plane, std::span<const Triangle> tris, BSP *parent);

public:
    // these nodes contain the child tris behind and in front of this
    std::unique_ptr<BSP> behind = nullptr;
    std::unique_ptr<BSP> in_front = nullptr;
    BSP *parent = nullptr;

    explicit BSP(std::span<const RenderPolygon> polys);

    void render(const MapEntity &parent, Frame &frame, const Camera &cam,
                std::span<const Texture> texs) const;
    int32_t n_nodes() const;
    int32_t n_triangles() const;
    int32_t max_depth() const;
    bool is_leaf() const;
    // get the leaf node a point is in
    const BSP &get_point_node(const Vec3 &point,
                              const MapEntity &parent_entity) const;
    BSP &get_point_node(const Vec3 &point, const MapEntity &parent_entity);
    bool point_in_solid(const Vec3 &point,
                        const MapEntity &parent_entity) const;
};
