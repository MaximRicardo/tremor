#include "bsp.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>

namespace {

constexpr float split_plane_epsilon = 0.0001f;

} // namespace

BSP::BSP(BSP *parent) : parent(parent) {}

BSP::BSP(Triangle node_tri, BSP *parent)
    : innode_info(new InNodeInfo), parent(parent)
{
    this->innode_info->tris.push_back(node_tri);
    this->innode_info->plane = node_tri.get_plane();
}

BSP::BSP(std::span<const Triangle> tris, BSP *parent) : parent(parent)
{
    if (tris.size() == 0)
        return;

    this->innode_info = std::make_unique<InNodeInfo>();
    this->innode_info->tris.push_back(tris[0]);
    this->innode_info->plane = tris[0].get_plane();

    std::vector<Triangle> other_tris;
    other_tris.reserve(tris.size() - 1);
    for (size_t i = 1; i < tris.size(); ++i) {
        other_tris.push_back(tris[i]);
    }

    this->insert(other_tris);
}

void BSP::insert_tris_behind(const Triangle &tri)
{
    auto tris_behind = Plane(-this->innode_info->plane.normal,
                             -this->innode_info->plane.d + split_plane_epsilon)
                           .clip(tri);

    if (tris_behind.n_tris == 0)
        return;

    size_t start_idx = 0;
    if (!this->behind) {
        this->behind = std::make_unique<BSP>(tris_behind.tris[0], this);
        start_idx = 1;
    }

    for (size_t i = start_idx; i < tris_behind.n_tris; ++i) {
        this->behind->insert(tris_behind.tris[i]);
    }
}

void BSP::insert_tris_in_front(const Triangle &tri)
{
    auto tris_in_front = Plane(this->innode_info->plane.normal,
                               this->innode_info->plane.d + split_plane_epsilon)
                             .clip(tri);

    if (tris_in_front.n_tris == 0)
        return;

    size_t start_idx = 0;
    if (!this->in_front) {
        this->in_front = std::make_unique<BSP>(tris_in_front.tris[0], this);
        start_idx = 1;
    }

    for (size_t i = start_idx; i < tris_in_front.n_tris; ++i) {
        this->in_front->insert(tris_in_front.tris[i]);
    }
}

void BSP::insert(const Triangle &tri)
{
    if (this->innode_info->plane.is_coplanar(tri.get_plane())) {
        this->innode_info->tris.push_back(tri);
    } else {
        this->insert_tris_behind(tri);
        this->insert_tris_in_front(tri);
    }
}

void BSP::insert(const std::span<Triangle> &tris)
{
    for (auto &tri : tris) {
        this->insert(tri);
    }
}

// probably gonna wanna find a way to make this function iterative at some point
void BSP::render(std::span<Color> frame, std::span<float> depth_buffer,
                 const Camera &cam, std::span<const Texture> texs) const
{
    if (this->is_leaf())
        return;

    if (!cam.get_frustum().b_box_partially_inside(this->b_box))
        return;

    bool cam_in_front = !this->innode_info->plane.is_point_behind(cam.pos);

    // since we wanna render everything back to front, we gotta flip around
    // behind and in front depending on whether the camera's definition of in
    // front agrees with the current node or not.
    auto &first = cam_in_front ? this->behind : this->in_front;
    auto &last = cam_in_front ? this->in_front : this->behind;

    if (first)
        first->render(frame, depth_buffer, cam, texs);

    if (cam_in_front) {
        for (const auto &tri : this->innode_info->tris) {
            tri.render(frame, depth_buffer, cam, texs);
        }
    }

    if (last)
        last->render(frame, depth_buffer, cam, texs);
}

size_t BSP::n_triangles() const
{
    if (this->is_leaf())
        return 0;

    size_t count = this->innode_info->tris.size();

    count += this->behind->n_triangles();
    count += this->in_front->n_triangles();

    return count;
}

size_t BSP::max_depth() const
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
    assert(this->innode_info);

    // std::make_unique can't access the private BSP constructor, so gonna have
    // to use new
    if (!this->behind)
        this->behind = std::unique_ptr<BSP>(new BSP(this));
    if (!this->in_front)
        this->in_front = std::unique_ptr<BSP>(new BSP(this));
}

void BSP::init_leaf_node()
{
    this->leaf_info = std::make_unique<LeafInfo>();

    // leaf nodes behind their parent are always in solid space, while ones
    // in front of their parents are always in empty space
    this->leaf_info->empty = this->parent->behind.get() != this;
}

void BSP::create_leaf_nodes(const AABB &cur_box)
{
    this->b_box = cur_box;

    if (this->is_leaf() && !this->innode_info) {
        this->init_leaf_node();
    } else {
        this->alloc_leaf_nodes();

        AABB behind_box = cur_box;
        behind_box.clip(this->innode_info->plane.flipped());
        AABB in_front_box = cur_box;
        in_front_box.clip(this->innode_info->plane);
        this->behind->create_leaf_nodes(behind_box);
        this->in_front->create_leaf_nodes(in_front_box);
    }
}

void BSP::create_leaf_nodes()
{
    this->create_leaf_nodes(AABB(
        Vec3(Consts::map_bounding_box_min_x, Consts::map_bounding_box_min_y,
             Consts::map_bounding_box_min_z),
        Vec3(Consts::map_bounding_box_max_x, Consts::map_bounding_box_max_y,
             Consts::map_bounding_box_max_z)));
}

const BSP &BSP::get_point_node(const Vec3 &point) const
{
    if (this->is_leaf()) {
        assert(this->leaf_info);
        return *this;
    } else {
        auto &node = this->innode_info->plane.is_point_behind(point)
                         ? this->behind
                         : this->in_front;
        return node->get_point_node(point);
    }
}

BSP &BSP::get_point_node(const Vec3 &point)
{
    return const_cast<BSP &>(std::as_const(*this).get_point_node(point));
}

bool BSP::point_in_solid(const Vec3 &point) const
{
    auto &p_node = this->get_point_node(point);

    std::cout << "point = (" << point << ")\n";
    std::cout << "min = (" << p_node.b_box.min << ")\n";
    std::cout << "max = (" << p_node.b_box.max << ")\n";
    std::cout << "point inside = " << p_node.b_box.is_point_inside(point)
              << "\n";
    return !p_node.leaf_info->empty;
}
