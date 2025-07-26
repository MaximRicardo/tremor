#include "bsp.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "map.hpp"
#include "mat4x4.hpp"
#include "plane.hpp"
#include "polygon.hpp"
#include "shape.hpp"
#include "triangle.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <variant>

namespace {

constexpr bool do_frustum_culling = true;
constexpr float split_plane_epsilon = 0.0001f;
constexpr float min_poly_area = 0.f;
// if false, the polygons in each leaf_info().edge_tris will be rendered instead
// of the polygons in each innode_info().tris.
constexpr bool render_innodes = true;

Camera get_rel_cam(const Camera &cam, const Matrix4x4 &inv_transform)
{
    Camera rel_cam = cam;
    rel_cam.pos = inv_transform * Vec4(cam.pos, 1.f);
    return rel_cam;
}

} // namespace

BSP::BSP(BSP *parent) : parent(parent) {}

BSP::BSP(const Plane &plane, std::span<const Triangle> tris, BSP *parent)
    : parent(parent)
{
    this->info = std::make_unique<InNodeInfo>();

    this->innode_info().plane = plane;
    this->innode_info().tris.assign(tris.begin(), tris.end());
}

BSP::BSP(std::span<const RenderPolygon> polys)
{
    assert(polys.size() > 0);

    std::cout << "constructing bsp\n";

    this->info = std::make_unique<InNodeInfo>();
    this->innode_info().plane = polys[0].get_plane();

    std::vector<RenderPolygon> other_polys;
    other_polys.assign(polys.begin() + 1, polys.end());
    this->create_outline(other_polys);

    std::cout << "done creating bsp\n";
    std::cout << "n nodes = " << this->n_nodes() << "\n";
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

void BSP::insert_poly_behind(const RenderPolygon &poly)
{
    auto behind_poly = poly;
    behind_poly.clip(Plane(-this->innode_info().plane.normal,
                           -this->innode_info().plane.d + split_plane_epsilon));

    if (behind_poly.empty())
        return;
    assert(!behind_poly.invalid());

    if (behind_poly.get_area() <= min_poly_area)
        return;

    if (!this->behind) {
        this->behind = std::unique_ptr<BSP>(new BSP(
            behind_poly.get_plane(), behind_poly.get_triangles(), this));
    } else {
        this->behind->insert(behind_poly);
    }
}

void BSP::insert_poly_in_front(const RenderPolygon &poly)
{
    auto in_front_poly = poly;
    in_front_poly.clip(
        Plane(this->innode_info().plane.normal,
              this->innode_info().plane.d + split_plane_epsilon));

    if (in_front_poly.empty())
        return;
    assert(!in_front_poly.invalid());

    if (in_front_poly.get_area() <= min_poly_area)
        return;

    if (!this->in_front) {
        this->in_front = std::unique_ptr<BSP>(
            new BSP(poly.get_plane(), poly.get_triangles(), this));
    } else {
        this->in_front->insert(in_front_poly);
    }
}

void BSP::insert(const RenderPolygon &poly)
{
    if (poly.get_area() <= min_poly_area)
        return;

    if (!this->innode_info().plane.is_coplanar(poly.get_plane())) {
        this->insert_poly_behind(poly);
        this->insert_poly_in_front(poly);
    } else {
        auto p_tris = poly.get_triangles();
        this->innode_info().tris.insert(this->innode_info().tris.end(),
                                        p_tris.begin(), p_tris.end());
    }
}

void BSP::create_outline(std::span<const RenderPolygon> tris)
{
    for (const auto &tri : tris) {
        this->insert(tri);
    }

    // a seperate pass is required to allocate all the leaf nodes once we know
    // we're done inserting triangles
    this->create_leaf_nodes();
}

void BSP::render_innode_tris(const MapEntity &parent, Frame &frame,
                             const Camera &cam,
                             std::span<const Texture> texs) const
{
    Camera rel_cam = get_rel_cam(cam, parent.get_inv_transform());

    for (const auto &tri : this->innode_info().tris) {
        if (tri.get_plane().is_point_behind(rel_cam.pos))
            continue;
        tri.render(parent.get_transform(), frame, cam, texs);
    }
}

void BSP::render_leaf_node_tris(const MapEntity &parent, Frame &frame,
                                const Camera &cam,
                                std::span<const Texture> texs) const
{
    Camera rel_cam = get_rel_cam(cam, parent.get_inv_transform());

    for (const auto &tri : this->leaf_info().edge_tris) {
        if (tri->get_plane().is_point_behind(rel_cam.pos))
            continue;
        tri->render(parent.get_transform(), frame, cam, texs);
    }
}

void BSP::render(const MapEntity &parent, Frame &frame, const Camera &cam,
                 std::span<const Texture> texs) const
{
    Camera rel_cam = get_rel_cam(cam, parent.get_inv_transform());

    if (this->is_leaf()) {
        if (!render_innodes)
            this->render_leaf_node_tris(parent, frame, cam, texs);
        return;
    }

    if (do_frustum_culling &&
        !rel_cam.get_frustum().maybe_partially_contains(this->b_box)) {
        return;
    }

    bool cam_in_front = !this->innode_info().plane.is_point_behind(rel_cam.pos);

    // everything is rendered back to front
    auto &first = cam_in_front ? this->behind : this->in_front;
    auto &last = cam_in_front ? this->in_front : this->behind;

    if (first)
        first->render(parent, frame, cam, texs);

    if (render_innodes) {
        this->render_innode_tris(parent, frame, cam, texs);
    }

    if (last)
        last->render(parent, frame, cam, texs);
}

int32_t BSP::n_nodes() const
{
    int32_t count = 1;

    if (this->behind)
        count += this->behind->n_nodes();
    if (this->in_front)
        count += this->in_front->n_nodes();

    return count;
}

int32_t BSP::n_triangles() const
{
    if (this->is_leaf()) {
        if (!render_innodes)
            return this->leaf_info().edge_tris.size();
        else
            return 0;
    } else {
        int32_t count = render_innodes ? this->innode_info().tris.size() : 0;

        count += this->behind->n_triangles();
        count += this->in_front->n_triangles();

        return count;
    }
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

void BSP::init_leaf_node(const std::vector<Triangle *> &tris)
{
    bool is_behind = this == this->parent->behind.get();

    this->info = std::make_unique<LeafInfo>();

    // leaf nodes behind their parent are always in solid space, while ones
    // in front of their parents are always in empty space
    this->leaf_info().empty = this->parent->behind.get() != this;

    for (auto ptri : tris) {
        /*
        bool skip = (is_behind && this->parent->innode_info().plane.normal.dot(
                                      ptri->get_plane().normal) > 0.f) ||
                    (!is_behind && this->parent->innode_info().plane.normal.dot(
                                       ptri->get_plane().normal) < 0.f);
        if (skip)
            continue;
            */
        const auto &par_p = this->parent->innode_info().plane;
        bool v0_b = par_p.is_point_behind(ptri->vs[0]);
        bool v1_b = par_p.is_point_behind(ptri->vs[1]);
        bool v2_b = par_p.is_point_behind(ptri->vs[2]);
        bool tri_behind = v0_b && v1_b && v2_b;
        bool intersects = v0_b != v1_b || v0_b != v2_b || v1_b != v2_b;
        if (!intersects &&
            ((is_behind && !tri_behind) || (!is_behind && tri_behind)))
            continue;

        this->leaf_info().edge_tris.push_back(ptri);
    }
}

// IF PERFORMANCE BECOMES AN ISSUE, CHECK IF COMPILER CONVERTS THE TRIS VECTOR
// TO A REFERENCE!
void BSP::create_leaf_nodes(const ConvexShape &cur_hull,
                            std::vector<Triangle *> tris)
{
    if (!cur_hull.polys.empty()) {
        this->b_box = cur_hull.get_aabb();
        this->b_box.min -= Vec3(16.f, 16.f, 16.f);
        this->b_box.max += Vec3(16.f, 16.f, 16.f);
    } else {
        // temporary fix
        this->b_box = AABB::map_box();
    }

    if (this->is_leaf() && !this->has_innode_info()) {
        this->init_leaf_node(tris);
    } else {
        this->alloc_leaf_nodes();

        for (auto &tri : this->innode_info().tris) {
            tris.push_back(&tri);
        }

        auto behind_hull = cur_hull;
        auto in_front_hull = cur_hull;
        if (!cur_hull.polys.empty()) { // temporary fix
            behind_hull.clip(this->innode_info().plane.flipped());
            in_front_hull.clip(this->innode_info().plane);
        }

        this->behind->create_leaf_nodes(behind_hull, tris);
        this->in_front->create_leaf_nodes(in_front_hull, tris);
    }
}

void BSP::create_leaf_nodes()
{
    auto world_hull = ConvexShape::box(
        Vec3(Consts::map_bounding_box_max_x - Consts::map_bounding_box_min_x,
             Consts::map_bounding_box_max_y - Consts::map_bounding_box_min_y,
             Consts::map_bounding_box_max_z - Consts::map_bounding_box_min_z));

    std::vector<Triangle *> ignore;
    this->create_leaf_nodes(world_hull, ignore);
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
