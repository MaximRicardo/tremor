#pragma once

#include "frame.hpp"
#include "mat4x4.hpp"
#include "plane.hpp"
#include "polygon.hpp"
#include "pvs.hpp"
#include "shape.hpp"
#include "ssize.hpp"
#include "vector/vec3.hpp"
#include <memory>
#include <span>
#include <variant>
#include <vector>

class MapEntity;
class BSPTree;

inline bool render_portals = false;

// a quake-style BSP, with polygons stored on innodes, and leaf nodes holding
// references to polygons which lie on their boundaries.
// TODO:
//    HEAVILY consider making a seperate class for leaf nodes and innodes.
//    there ain't no way this is good design bruh.
class BSP {

    // i honestly don't know if this is cursed or not, but it works
    class LeafInfo {
    public:
        // triangles lying on the edge of the node's hull.
        std::vector<Triangle *> edge_tris;
        bool rendered = false;

        // these are used for PVS
        std::vector<BSP *> pv_leaves;
        std::vector<PVS::Portal> portals;

        bool solid() const;
    };

    class InNodeInfo {
    public:
        Plane plane;
        // triangles which are coplanar with plane
        std::vector<Triangle> tris;
        isize_t brush_id; // the id of the brush the first triangle in tris came
                          // from. can be any number so long as it is unique for
                          // every brush.
    };

    AABB b_box;
    ConvexShape shape = ConvexShape::box(Vec3(1.f, 1.f, 1.f));

    std::variant<std::unique_ptr<LeafInfo>, std::unique_ptr<InNodeInfo>> info;

    bool has_innode_info() const;
    bool has_leaf_info() const;

    const InNodeInfo &innode_info() const;
    InNodeInfo &innode_info();
    const LeafInfo &leaf_info() const;
    LeafInfo &leaf_info();

    void alloc_leaf_nodes(BSPTree &parent);
    void init_leaf_node(const std::vector<Triangle *> &tris);
    void create_leaf_nodes(const ConvexShape &cur_hull,
                           std::vector<Triangle *> tris, BSPTree &parent);
    void create_leaf_nodes(BSPTree &parent);

    // doesn't physically put triangles into the tree, instead uses the provided
    // triangles' planes to create the structure of the tree
    void insert_poly_behind(const RenderPolygon &poly, isize_t brush_id);
    void insert_poly_in_front(const RenderPolygon &poly, isize_t brush_id);
    void insert(const RenderPolygon &poly, isize_t brush_id);

    void render_innode_tris(const MapEntity &parent, Frame &frame,
                            const Camera &cam, std::span<const Texture> texs,
                            isize_t &cur_sort_key);
    void render_leaf_node_tris(const MapEntity &parent, Frame &frame,
                               const Camera &cam, std::span<const Texture> texs,
                               isize_t &cur_sort_key);
    void render_leaf_portals(const MapEntity &parent, Frame &frame,
                             const Camera &cam,
                             std::span<const Texture> texs) const;
    void render(const MapEntity &parent, Frame &frame, const Camera &cam,
                std::span<const Texture> texs, isize_t &cur_sort_key);

    const BSP &get_point_node(const Vec3 &point,
                              const Matrix4x4 &point_trnsfrm) const;
    BSP &get_point_node(const Vec3 &point, const Matrix4x4 &point_trnsfrm);
    std::vector<const Triangle *> leaf_tris_on_plane(const Plane &plane) const;
    std::vector<Triangle *> leaf_tris_on_plane(const Plane &plane);
    void new_frame();
    void create_portals();
    void merge_portal(const PVS::Portal &p);
    void find_pv_leaves();
    // cur is the portal leading to the current node
    void add_visibles(const PVS::Portal &cur, const Polygon &pass,
                      const Polygon &start);

    explicit BSP(BSP *parent);
    // innode constructor
    BSP(const Plane &plane, std::span<const Triangle> tris, isize_t brush_id,
        BSP *parent);

public:
    // these nodes contain the child tris behind and in front of this
    std::unique_ptr<BSP> behind = nullptr;
    std::unique_ptr<BSP> in_front = nullptr;
    BSP *parent = nullptr;

    explicit BSP(std::span<const BrushShape> brushes, BSPTree &parent);

    void render(const MapEntity &parent, Frame &frame, const Camera &cam,
                std::span<const Texture> texs);
    isize_t n_nodes() const;
    isize_t n_triangles() const;
    isize_t max_depth() const;
    bool is_leaf() const;
    bool is_innode() const;
    bool contains(const Vec3 &p) const;
    // get the leaf node a point is in
    const BSP &get_point_node(const Vec3 &point,
                              const MapEntity &parent_entity) const;
    BSP &get_point_node(const Vec3 &point, const MapEntity &parent_entity);
    bool point_in_solid(const Vec3 &point,
                        const MapEntity &parent_entity) const;
    // returns whichever node contains a plane coplanar to the given plane
    const BSP *get_plane_node(const Plane &plane) const;
    BSP *get_plane_node(const Plane &plane);
    const BSP &closest_node(const Vec3 &v) const;
    BSP &closest_node(const Vec3 &v);

    friend BSPTree;
};

class BSPTree {

    std::vector<BSP *> leaves; // IMPORTANT THAT THIS COMES BEFORE ROOT FOR
                               // INITIALIZATION ORDER!
    std::unique_ptr<BSP> root;

    // put these in BSP instead?
    std::vector<BSP *> get_leaf_neighbors(const BSP &leaf);
    /*
    void create_leaf_portals(BSP &leaf, std::span<BSP *> neighbors);
    void create_leaf_portals(BSP &leaf, BSP &other, const Plane &boundary);
    */

    void merge_portals();
    void remove_useless_portals();
    void resize_portals();
    void calc_pvs();
    isize_t leaf_idx(const BSP &leaf) const;

public:
    explicit BSPTree(std::span<const BrushShape> polys);

    const BSP &get_root() const;
    BSP &get_root();
    void render(const MapEntity &parent, Frame &frame, const Camera &cam,
                std::span<const Texture> texs);

    friend BSP;
};
