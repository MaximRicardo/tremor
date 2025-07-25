#include "bsp.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "map.hpp"
#include "mat4x4.hpp"
#include "shape.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <variant>

namespace {

constexpr bool place_tris_in_solid_nodes = true;
constexpr bool do_frustum_culling = true;
constexpr float split_plane_epsilon = 0.0001f;
constexpr float min_triangle_area = 0.f;

} // namespace

BSP::BSP(BSP *parent) : parent(parent) {}

BSP::BSP(const Plane &plane, BSP *parent) : parent(parent)
{
    this->info = std::make_unique<InNodeInfo>();

    this->innode_info().plane = plane;
}

// works by first creating the outline of the tree, and then filling the tree
// with actual triangles. this is cuz we need full knowledge of the bsp tree's
// splitting planes before we can start sending triangles to the leaf nodes,
// splitting them with every innode's splitting plane along the way.
BSP::BSP(std::span<const Triangle> tris)
{
    assert(tris.size() > 0);

    std::cout << "constructing bsp\n";

    this->info = std::make_unique<InNodeInfo>();
    this->innode_info().plane = tris[0].get_plane();

    std::vector<Triangle> other_tris;
    other_tris.assign(tris.begin() + 1, tris.end());
    this->create_outline(other_tris);

    std::cout << "done creating outline bsp\n";

    this->fill_with_triangles(tris);

    std::cout << "done constructing bsp\n";
}

bool BSP::has_innode_info() const
{
    return std::holds_alternative<std::unique_ptr<InNodeInfo>>(this->info);
}

bool BSP::has_leaf_info() const
{
    return std::holds_alternative<std::unique_ptr<LeafInfo>>(this->info);
}

const BSP::InNodeInfo &BSP::innode_info() const
{
    assert(this->has_innode_info());
    return *std::get<std::unique_ptr<InNodeInfo>>(this->info);
}

BSP::InNodeInfo &BSP::innode_info()
{
    return const_cast<BSP::InNodeInfo &>(std::as_const(*this).innode_info());
}

const BSP::LeafInfo &BSP::leaf_info() const
{
    assert(this->has_leaf_info());
    return *std::get<std::unique_ptr<LeafInfo>>(this->info);
}

BSP::LeafInfo &BSP::leaf_info()
{
    return const_cast<LeafInfo &>(std::as_const(*this).leaf_info());
}

void BSP::insert_tris_behind(const Triangle &tri, bool leaf_insert)
{
    auto &info = this->innode_info();

    auto tris_behind =
        Plane(-info.plane.normal, -info.plane.d + split_plane_epsilon)
            .clip(tri);

    if (tris_behind.n_tris == 0)
        return;

    size_t start_idx = 0;
    if (tris_behind.tris[start_idx].get_area() <= min_triangle_area)
        ++start_idx;

    if (!this->behind) {
        this->behind = std::unique_ptr<BSP>(
            new BSP(tris_behind.tris[start_idx].get_plane(), this));
        ++start_idx;
    }

    /*
    std::cout << "n in behind = " << tris_behind.n_tris << "\n";
    */

    for (size_t i = start_idx; i < tris_behind.n_tris; ++i) {
        if (leaf_insert)
            this->behind->leaf_insert(tris_behind.tris[i]);
        else
            this->behind->insert(tris_behind.tris[i]);
    }
}

void BSP::insert_tris_in_front(const Triangle &tri, bool leaf_insert)
{
    auto tris_in_front =
        Plane(this->innode_info().plane.normal,
              this->innode_info().plane.d + split_plane_epsilon)
            .clip(tri);

    if (tris_in_front.n_tris == 0)
        return;

    size_t start_idx = 0;
    if (tris_in_front.tris[start_idx].get_area() <= min_triangle_area)
        ++start_idx;

    if (!this->in_front) {
        this->in_front = std::unique_ptr<BSP>(
            new BSP(tris_in_front.tris[start_idx].get_plane(), this));
        ++start_idx;
    }

    /*
    std::cout << "n in front = " << tris_in_front.n_tris << "\n";
    */

    for (size_t i = start_idx; i < tris_in_front.n_tris; ++i) {
        if (leaf_insert)
            this->in_front->leaf_insert(tris_in_front.tris[i]);
        else
            this->in_front->insert(tris_in_front.tris[i]);
    }
}

void BSP::insert(const Triangle &tri)
{
    if (tri.get_area() <= min_triangle_area)
        return;

    /*
    std::cout << "tri vs = [(" << tri.vs[0] << "), (" << tri.vs[1] << "), ("
              << tri.vs[2] << ")]\n";
              */

    if (!this->innode_info().plane.is_coplanar(tri.get_plane())) {
        this->insert_tris_behind(tri, false);
        this->insert_tris_in_front(tri, false);
    }
}

void BSP::create_outline(std::span<const Triangle> tris)
{
    for (const auto &tri : tris) {
        this->insert(tri);
    }

    // a seperate pass is required to allocate all the leaf nodes once we know
    // we're done inserting triangles
    this->create_leaf_nodes();
}

void BSP::leaf_insert(const Triangle &tri)
{
    if (tri.get_area() < min_triangle_area)
        return;

    if (this->is_leaf()) {
        // don't insert triangles into solid nodes, cuz those triangles will
        // never be seen anyway
        if (place_tris_in_solid_nodes || this->leaf_info().empty)
            this->leaf_info().edge_tris.push_back(tri);
    } else if (!this->innode_info().plane.is_coplanar(tri.get_plane())) {
        this->insert_tris_behind(tri, true);
        this->insert_tris_in_front(tri, true);
    } else if (this->innode_info().plane.normal.dot(tri.get_plane().normal) <
               0.f) {
        this->behind->leaf_insert(tri);
    } else {
        this->in_front->leaf_insert(tri);
    }
}

void BSP::fill_with_triangles(std::span<const Triangle> tris)
{
    for (const auto &tri : tris) {
        this->leaf_insert(tri);
    }
}

void BSP::render(const MapEntity &parent, Frame &frame, const Camera &cam,
                 std::span<const Texture> texs) const
{
    Camera rel_cam = cam;
    rel_cam.pos = parent.get_inv_transform() * Vec4(cam.pos, 1.f);

    if (this->is_leaf()) {
        for (const auto &tri : this->leaf_info().edge_tris) {
            if (tri.get_plane().is_point_behind(rel_cam.pos))
                continue;
            tri.render(parent.get_transform(), frame, cam, texs);
        }
        return;
    }

    if (do_frustum_culling &&
        !rel_cam.get_frustum().maybe_partially_contains(this->b_box)) {
        return;
    }

    bool cam_in_front = !this->innode_info().plane.is_point_behind(rel_cam.pos);

    // since we wanna render everything back to front, we gotta flip around
    // behind and in front depending on whether the camera's definition of in
    // front agrees with the current node or not.
    auto &first = cam_in_front ? this->behind : this->in_front;
    auto &last = cam_in_front ? this->in_front : this->behind;

    if (first)
        first->render(parent, frame, cam, texs);

    if (last)
        last->render(parent, frame, cam, texs);
}

int32_t BSP::n_triangles() const
{
    if (this->is_leaf())
        return this->leaf_info().edge_tris.size();

    int32_t count = 0;

    count += this->behind->n_triangles();
    count += this->in_front->n_triangles();

    return count;
}

int32_t BSP::max_depth() const
{
    if (this->is_leaf())
        return 1;

    return 1 + std::max(this->behind->max_depth(), this->in_front->max_depth());
}

bool BSP::is_leaf() const
{
    return !this->in_front && !this->behind;
}

void BSP::alloc_leaf_nodes()
{
    assert(this->has_innode_info());

    // std::make_unique can't access the private BSP constructor, so gonna have
    // to use new
    if (!this->behind)
        this->behind = std::unique_ptr<BSP>(new BSP(this));
    if (!this->in_front)
        this->in_front = std::unique_ptr<BSP>(new BSP(this));
}

void BSP::init_leaf_node()
{
    this->info = std::make_unique<LeafInfo>();

    // leaf nodes behind their parent are always in solid space, while ones
    // in front of their parents are always in empty space
    this->leaf_info().empty = this->parent->behind.get() != this;
}

void BSP::create_leaf_nodes(const ConvexShape &cur_hull)
{
    this->b_box = cur_hull.get_aabb();
    this->b_box.min -= Vec3(16.f, 16.f, 16.f);
    this->b_box.max += Vec3(16.f, 16.f, 16.f);

    if (this->is_leaf() && !this->has_innode_info()) {
        this->init_leaf_node();
    } else {
        this->alloc_leaf_nodes();

        auto behind_hull = cur_hull;
        behind_hull.clip(this->innode_info().plane.flipped());
        auto in_front_hull = cur_hull;
        in_front_hull.clip(this->innode_info().plane);
        this->behind->create_leaf_nodes(behind_hull);
        this->in_front->create_leaf_nodes(in_front_hull);
    }
}

void BSP::create_leaf_nodes()
{
    auto world_hull = ConvexShape::box(
        Vec3(Consts::map_bounding_box_max_x - Consts::map_bounding_box_min_x,
             Consts::map_bounding_box_max_y - Consts::map_bounding_box_min_y,
             Consts::map_bounding_box_max_z - Consts::map_bounding_box_min_z));

    this->create_leaf_nodes(world_hull);
}

const BSP &BSP::get_point_node(const Vec3 &point, const MapEntity &parent) const
{
    Vec3 rel_p = parent.get_inv_transform() * Vec4(point, 1.f);

    if (this->is_leaf()) {
        assert(this->has_leaf_info());
        return *this;
    } else {
        auto &node = this->innode_info().plane.is_point_behind(rel_p)
                         ? this->behind
                         : this->in_front;
        return node->get_point_node(point, parent);
    }
}

BSP &BSP::get_point_node(const Vec3 &point, const MapEntity &parent)
{
    return const_cast<BSP &>(
        std::as_const(*this).get_point_node(point, parent));
}

bool BSP::point_in_solid(const Vec3 &point, const MapEntity &parent) const
{
    auto &p_node = this->get_point_node(point, parent);

    std::cout << "point = (" << point << ")\n";
    std::cout << "min = (" << p_node.b_box.min << ")\n";
    std::cout << "max = (" << p_node.b_box.max << ")\n";
    bool inside =
        p_node.b_box.contains(parent.get_inv_transform() * Vec4(point, 1.f));
    std::cout << "point inside = " << inside << "\n";
    assert(inside);
    return !p_node.leaf_info().empty;
}
