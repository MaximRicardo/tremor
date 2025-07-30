#include "bsp.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "map.hpp"
#include "mat4x4.hpp"
#include "plane.hpp"
#include "polygon.hpp"
#include "pvs.hpp"
#include "render_px.hpp"
#include "rspan.hpp"
#include "shape.hpp"
#include "ssize.hpp"
#include "triangle.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <span>
#include <stack>
#include <utility>
#include <variant>
#include <vector>

namespace {

constexpr float split_plane_epsilon = 0.0001f;
constexpr float portal_width_epsilon = 0.1f;
constexpr float min_poly_area = 0.f;
constexpr bool do_frustum_culling = false;
// if false, the polygons in each leaf_info().edge_tris will be rendered instead
// of the polygons in each innode_info().tris.
constexpr bool render_innodes = true;
// constexpr bool render_portals = true;
constexpr bool optimize_tree = false;

Camera get_rel_cam(const Camera &cam, const Matrix4x4 &inv_transform)
{
    Camera rel_cam = cam;
    rel_cam.pos = inv_transform * Vec4(cam.pos, 1.f);
    return rel_cam;
}

int32_t n_polys_clipped(isize_t poly, std::span<const RenderPolygon> polys,
                        isize_t start)
{
    int32_t n = 0;

    auto p = polys[poly].get_plane();

    for (auto other = polys.begin() + start; other < polys.end(); ++other) {
        if (poly == std::distance(polys.begin(), other))
            continue;

        if (other->intersects(p))
            ++n;
    }

    return n;
}

isize_t get_best_polygon(std::span<const RenderPolygon> polys, isize_t start)
{
    isize_t best = -1;
    int32_t best_score = INT32_MIN;

    for (auto i = polys.begin() + start; i < polys.end(); ++i) {
        isize_t idx = std::distance(polys.begin(), i);

        int32_t n_clipped = n_polys_clipped(idx, polys, start);

        int32_t score = -n_clipped;
        if (!best || score > best_score) {
            best = idx;
            best_score = score;
        }
    }

    assert(best != -1);
    return best;
}

std::vector<const RenderPolygon *>
sort_by_score(std::span<const RenderPolygon> polys)
{
    std::vector<const RenderPolygon *> sorted;
    for (const auto &poly : polys)
        sorted.push_back(&poly);

    if (optimize_tree) {
        for (size_t i = 0; i < sorted.size(); ++i) {
            size_t best = get_best_polygon(polys, i);
            std::swap(sorted[i], sorted[best]);
        }
    }

    return sorted;
}

/*
BSP *node_containing_p(const Vec3 &p, std::span<BSP *> nodes)
{
    for (auto node : nodes) {
        if (node->contains(p))
            return node;
    }

    return nullptr;
}

// returns the first one
const Polygon *poly_on_plane(std::span<const Polygon> polys, const Plane &plane)
{
    for (const auto &poly : polys) {
        if (poly.is_on(plane, 0.1f)) {
            return &poly;
        }
    }

    return nullptr;
}
*/

/*
bool poly_visible(const Polygon &poly, const Matrix4x4 &transform,
                  const Frame &frame, const Camera &cam)
{
    auto tris = poly.get_triangles();

    for (const auto &tri : tris) {
        if (tri.is_visible(transform, frame, cam))
            return true;
    }

    return false;
}
*/

} // namespace

BSP::BSP(BSP *parent) : parent(parent) {}

BSP::BSP(const Plane &plane, std::span<const Triangle> tris, BSP *parent)
    : parent(parent)
{
    this->info = std::make_unique<InNodeInfo>();

    this->innode_info().plane = plane;
    this->innode_info().tris.assign(tris.begin(), tris.end());
}

BSP::BSP(std::span<const RenderPolygon> og_polys, BSPTree &parent)
{
    assert(og_polys.size() > 0);

    std::cout << "constructing bsp\n";

    auto sorted = sort_by_score(og_polys);
    std::cout << "done sorting\n";

    this->info = std::make_unique<InNodeInfo>();
    this->innode_info().plane = sorted[0]->get_plane();

    std::vector<RenderPolygon> others;
    for (const auto &poly : sorted)
        others.push_back(*poly);

    for (const auto &poly : others) {
        this->insert(poly);
    }

    // a seperate pass is required to allocate all the leaf nodes once we know
    // we're done inserting triangles
    this->create_leaf_nodes(parent);

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

void BSP::render_innode_tris(const MapEntity &parent, Frame &frame,
                             const Camera &cam, std::span<const Texture> texs,
                             isize_t &cur_sort_key)
{
    Camera rel_cam = get_rel_cam(cam, parent.get_inv_transform());

    for (auto &tri : this->innode_info().tris) {
        if (tri.get_plane().is_point_behind(rel_cam.pos))
            continue;
        tri.sort_key = cur_sort_key++;
        tri.render(parent.get_transform(), frame, cam, texs);
    }
}

void BSP::render_leaf_node_tris(const MapEntity &parent, Frame &frame,
                                const Camera &cam,
                                std::span<const Texture> texs,
                                isize_t &cur_sort_key)
{
    Camera rel_cam = get_rel_cam(cam, parent.get_inv_transform());

    for (auto &tri : this->leaf_info().edge_tris) {
        if (tri->get_plane().is_point_behind(rel_cam.pos))
            continue;
        // tri was already rendered in front of its current position
        if (tri->sort_key >= 0 && tri->sort_key < cur_sort_key)
            continue;
        tri->sort_key = cur_sort_key++;
        tri->render(parent.get_transform(), frame, cam, texs);
    }

    if (render_portals)
        this->render_leaf_portals(parent, frame, cam, texs);
}

void BSP::render_leaf_portals(const MapEntity &parent, Frame &frame,
                              const Camera &cam,
                              std::span<const Texture> texs) const
{
    if (render_portals) {
        for (const auto &portal : this->leaf_info().portals) {
            auto tris = portal.shape.get_triangles();
            for (const auto &tri : tris) {
                tri.render(parent.get_transform(), frame, cam, texs);
            }
        }
    }
}

void BSP::render(const MapEntity &parent, Frame &frame, const Camera &cam,
                 std::span<const Texture> texs, isize_t &cur_sort_key)
{
    Camera rel_cam = get_rel_cam(cam, parent.get_inv_transform());

    if (do_frustum_culling &&
        !rel_cam.get_frustum().maybe_partially_contains(this->b_box)) {
        return;
    }

    if (this->is_leaf()) {
        if (render_innodes)
            this->render_leaf_node_tris(parent, frame, cam, texs, cur_sort_key);
        return;
    }

    bool cam_in_front = !this->innode_info().plane.is_point_behind(rel_cam.pos);

    // everything is rendered front to back
    auto first = cam_in_front ? this->in_front.get() : this->behind.get();
    auto last = cam_in_front ? this->behind.get() : this->in_front.get();
    // unless spans are disabled, in which case they're not
    if (!RSpan::enabled)
        std::swap(first, last);

    if (first)
        first->render(parent, frame, cam, texs, cur_sort_key);
    if (!render_innodes)
        this->render_innode_tris(parent, frame, cam, texs, cur_sort_key);
    if (last)
        last->render(parent, frame, cam, texs, cur_sort_key);
}

void BSP::render(const MapEntity &parent, Frame &frame, const Camera &cam,
                 std::span<const Texture> texs)
{
    isize_t start_key = 0;
    this->render(parent, frame, cam, texs, start_key);
}

isize_t BSP::n_nodes() const
{
    int32_t count = 1;

    if (this->behind)
        count += this->behind->n_nodes();
    if (this->in_front)
        count += this->in_front->n_nodes();

    return count;
}

isize_t BSP::n_triangles() const
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

isize_t BSP::max_depth() const
{
    if (this->is_leaf())
        return 1;

    return 1 + std::max(this->behind->max_depth(), this->in_front->max_depth());
}

bool BSP::is_leaf() const
{
    return !this->in_front && !this->behind;
}

bool BSP::is_innode() const
{
    return !this->is_leaf();
}

void BSP::alloc_leaf_nodes(BSPTree &parent)
{
    assert(this->has_innode_info());

    // std::make_unique can't access the private BSP constructor, so gonna have
    // to use new
    if (!this->behind) {
        this->behind = std::unique_ptr<BSP>(new BSP(this));
        parent.leaves.push_back(this->behind.get());
    }
    if (!this->in_front) {
        this->in_front = std::unique_ptr<BSP>(new BSP(this));
        parent.leaves.push_back(this->in_front.get());
    }
}

void BSP::init_leaf_node(const std::vector<Triangle *> &tris)
{
    // bool is_behind = this == this->parent->behind.get();

    this->info = std::make_unique<LeafInfo>();

    // leaf nodes behind their parent are always in solid space, while ones
    // in front of their parents are always in empty space
    this->leaf_info().empty = this->parent->behind.get() != this;

    for (auto tri : tris) {
        this->leaf_info().edge_tris.push_back(tri);
    }

    this->create_portals();
}

// IF PERFORMANCE BECOMES AN ISSUE, CHECK IF COMPILER CONVERTS THE TRIS VECTOR
// TO A REFERENCE!
void BSP::create_leaf_nodes(const ConvexShape &cur_hull,
                            std::vector<Triangle *> tris, BSPTree &parent)
{
    this->b_box = cur_hull.get_aabb();
    this->shape = cur_hull;

    if (this->is_leaf() && !this->has_innode_info()) {
        this->init_leaf_node(tris);
    } else {
        if (this->parent) {
            bool is_behind = this->parent->behind.get() == this;
            std::erase_if(tris, [this, is_behind](const auto &tri) {
                bool is_on = tri->is_on(this->innode_info().plane, 1.f);
                if (is_behind)
                    return tri->is_in_front(this->innode_info().plane) &&
                           !is_on;
                else
                    return tri->is_behind(this->innode_info().plane) && !is_on;
            });
        }

        this->alloc_leaf_nodes(parent);

        for (auto &tri : this->innode_info().tris) {
            tris.push_back(&tri);
        }

        auto behind_hull = cur_hull;
        auto in_front_hull = cur_hull;
        behind_hull.clip(this->innode_info().plane.flipped());
        in_front_hull.clip(this->innode_info().plane);

        this->behind->create_leaf_nodes(behind_hull, tris, parent);
        this->in_front->create_leaf_nodes(in_front_hull, tris, parent);
    }
}

void BSP::create_leaf_nodes(BSPTree &parent)
{
    auto world_hull = ConvexShape::box(
        Vec3(Consts::map_bounding_box_max - Consts::map_bounding_box_min,
             Consts::map_bounding_box_max - Consts::map_bounding_box_min,
             Consts::map_bounding_box_max - Consts::map_bounding_box_min));

    std::vector<Triangle *> ignore;
    this->create_leaf_nodes(world_hull, ignore, parent);
}

bool BSP::contains(const Vec3 &p) const
{
    return this->shape.contains(p);
}

const BSP &BSP::get_point_node(const Vec3 &point,
                               const Matrix4x4 &point_trnsfrm) const
{
    Vec3 rel_p = point_trnsfrm * Vec4(point, 1.f);

    if (this->is_leaf()) {
        assert(this->has_leaf_info());
        return *this;
    } else {
        auto &node = this->innode_info().plane.is_point_behind(rel_p)
                         ? this->behind
                         : this->in_front;
        return node->get_point_node(point, point_trnsfrm);
    }
}

BSP &BSP::get_point_node(const Vec3 &point, const Matrix4x4 &point_trnsfrm)
{
    return const_cast<BSP &>(
        std::as_const(*this).get_point_node(point, point_trnsfrm));
}

std::vector<const Triangle *> BSP::leaf_tris_on_plane(const Plane &plane) const
{
    std::vector<const Triangle *> ret;

    for (auto &tri : this->leaf_info().edge_tris) {
        if (tri->is_on(plane))
            ret.push_back(tri);
    }

    return ret;
}

const BSP &BSP::get_point_node(const Vec3 &point, const MapEntity &parent) const
{
    return this->get_point_node(point, parent.get_inv_transform());
}

void BSP::reset_sort_keys()
{
    if (this->is_leaf())
        return;

    for (auto &tri : this->innode_info().tris) {
        tri.sort_key = -1;
    }

    if (this->behind)
        this->behind->reset_sort_keys();
    if (this->in_front)
        this->in_front->reset_sort_keys();
}

void BSP::create_portals()
{
    assert(this->is_leaf());

    for (const auto &poly : this->shape.polys) {
        this->leaf_info().portals.emplace_back(poly, this, nullptr);
    }
}

void BSP::merge_portal(const PVS::Portal &other)
{
    assert(this->is_leaf());
    assert(!other.in_front);

    bool merged = false;
    auto &portals = this->leaf_info().portals;
    isize_t old_n = std::ssize(portals);
    for (isize_t i = 0; i < old_n; ++i) {
        if (portals[i].in_front)
            continue;
        else if (!portals[i].shape.partially_contains(other.shape))
            continue;

        PVS::Portal new_p = portals[i];
        new_p.merge_with(other);
        portals.push_back(new_p);
        merged = true;
    }

    (void)merged;
    // assert(merged);
}

void BSP::merge_portals(BSP &a, BSP &b)
{
    assert(&a != &b);
    if (!a.leaf_info().empty || !b.leaf_info().empty)
        return;

    std::vector<Polygon> new_portals;

    for (auto &a_portal : a.leaf_info().portals) {
        if (a_portal.in_front) {
            continue;
        } else if (a_portal.shape.get_area() < Consts::epsilon)
            continue;

        for (auto &b_portal : b.leaf_info().portals) {
            if (b_portal.in_front)
                continue;
            else if (b_portal.shape.get_area() < Consts::epsilon)
                continue;
            else if (!a_portal.shape.partially_contains(b_portal.shape)) {
                auto a_plane = a_portal.shape.get_plane();
                auto b_plane = b_portal.shape.get_plane();
                std::cout << "a_portal = [" << a_portal.shape << "]\n";
                std::cout << "a normal = (" << a_plane.normal
                          << "), d = " << a_plane.d << "\n";
                std::cout << "b_portal = [" << b_portal.shape << "]\n";
                std::cout << "b normal = (" << b_plane.normal
                          << "), d = " << b_plane.d << "\n";
                continue;
            }

            new_portals.emplace_back(a_portal.shape.get_area() <
                                             b_portal.shape.get_area()
                                         ? a_portal.shape
                                         : b_portal.shape);
        }
    }

    for (const auto &shape : new_portals) {
        a.leaf_info().portals.emplace_back(shape, &a, &b);
        b.leaf_info().portals.emplace_back(shape, &b, &a);

        bool a_behind = shape.get_plane().is_point_behind(a.b_box.get_center());
        if (a_behind)
            b.leaf_info().portals.back().shape.flip_dir();
        else
            a.leaf_info().portals.back().shape.flip_dir();
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

    bool inside =
        p_node.shape.contains(parent.get_inv_transform() * Vec4(point, 1.f));
    assert(inside);
    return !p_node.leaf_info().empty;
}

const BSP *BSP::get_plane_node(const Plane &plane) const
{
    if (this->is_leaf())
        return nullptr;
    else if (this->innode_info().plane.is_coplanar(plane))
        return this;

    /*
    std::cout << "plane normal = (" << plane.normal << "), d = " << plane.d
              << "\n";
    std::cout << "node normal = (" << this->innode_info().plane.normal
              << "), d = " << this->innode_info().plane.d << "\n";
              */

    auto a = this->behind->get_plane_node(plane);
    if (a)
        return a;

    auto b = this->in_front->get_plane_node(plane);
    if (b)
        return b;

    return nullptr;
}

BSP *BSP::get_plane_node(const Plane &plane)
{
    return const_cast<BSP *>(std::as_const(*this).get_plane_node(plane));
}

const BSP &BSP::closest_node(const Vec3 &v) const
{
    if (this->is_leaf())
        return *this;

    bool v_behind = this->innode_info().plane.is_point_behind(v);
    return v_behind ? this->behind->closest_node(v)
                    : this->in_front->closest_node(v);
}

BSP &BSP::closest_node(const Vec3 &v)
{
    return const_cast<BSP &>(std::as_const(*this).closest_node(v));
}

BSPTree::BSPTree(std::span<const RenderPolygon> polys)
    : root(new BSP(polys, *this))
{
    this->merge_portals();
    this->remove_useless_portals();
}

const BSP &BSPTree::get_root() const
{
    return *this->root;
}

BSP &BSPTree::get_root()
{
    return const_cast<BSP &>(std::as_const(*this).get_root());
}

std::vector<BSP *> BSPTree::get_leaf_neighbors(const BSP &leaf)
{
    std::vector<BSP *> neighbors;

    /*
    BSP *cur_node = leaf.parent;
    const BSP *prev_node = &leaf;
    isize_t n_neighbors = leaf.shape.polys.size();
    for (isize_t i = 0; i < n_neighbors; ++i) {
        bool from_behind = prev_node == cur_node->behind.get();
        BSP *neighbor =
            from_behind ? cur_node->in_front.get() : cur_node->behind.get();
        if (!neighbor->is_leaf())
            break;
        // if this assert goes off, try picking the child leaf closest to leaf
        // assert(neighbor->is_leaf());
        neighbors.push_back(neighbor);

        prev_node = cur_node;
        cur_node = cur_node->parent;
    }
    */

    // this can probably be optimized
    for (const auto &poly : leaf.shape.polys) {
        auto plane = poly.get_plane();

        Vec3 p = poly.get_center() + plane.normal * 0.1f;
        if (!this->root->b_box.contains(p))
            continue;

        auto &node = this->root->get_point_node(p, Matrix4x4::identity());

        assert(node.shape.contains(p));

        assert(node.is_leaf());
        if (std::find(neighbors.begin(), neighbors.end(), &node) ==
            neighbors.end())
            neighbors.push_back(&node);
    }

    return neighbors;
}

#if 0
void BSPTree::create_leaf_portals(BSP &leaf, std::span<BSP *> neighbors)
{
    for (const auto &poly : leaf.shape.polys) {
        auto plane = poly.get_plane();
        BSP *other = node_containing_p(poly.get_center() + plane.normal * 0.01f,
                                       neighbors);
        if (other /* && other->leaf_info().empty*/)
            this->create_leaf_portals(leaf, *other, poly.get_plane());
    }
}

void BSPTree::create_leaf_portals(BSP &a, BSP &b, const Plane &boundary)
{
    assert(!a.shape.polys.empty());
    assert(!b.shape.polys.empty());

    std::cout << "a center = " << a.b_box.get_center() << "\n";
    std::cout << "b center = " << b.b_box.get_center() << "\n";

    std::cout << "checking a\n";
    const Polygon *a_poly = poly_on_plane(a.shape.polys, boundary);
    std::cout << "checking b\n";
    const Polygon *b_poly = poly_on_plane(b.shape.polys, boundary);

    if (!a_poly)
        a_poly = &a.shape.polys[0];
    if (!b_poly)
        b_poly = &b.shape.polys[0];

    // i see absolutely no way this could ever go wrong! :)
    if (!a_poly || !b_poly)
        return;

    bool a_smallest = a_poly->get_area() < b_poly->get_area();
    const Polygon &smallest = a_smallest ? *a_poly : *b_poly;

    // the polygon always points outwards from it's node
    BSP &in_front = a_smallest ? b : a;
    BSP &behind = a_smallest ? a : b;

    PVS::Portal portal(smallest, &in_front, &behind);

    a.leaf_info().portals.push_back(portal);
    b.leaf_info().portals.push_back(portal);
}
#endif

void BSPTree::merge_portals()
{
    // pls work i'm at wits' end here
    for (auto leaf : this->leaves) {
        auto &portals = leaf->leaf_info().portals;
        isize_t old_n = std::ssize(portals);
        for (isize_t i = 0; i < old_n; ++i) {
            if (portals[i].in_front)
                continue;

            auto p_plane = portals[i].shape.get_plane();

            BSP *plane_parent = this->root->get_plane_node(p_plane);
            if (!plane_parent)
                continue;
            assert(plane_parent->is_innode());

            /*
            bool opposite = plane_parent->innode_info().plane.normal.dot(
                                p_plane.normal) < 0.f;
                                */

            Vec3 center = portals[i].shape.get_center();
            /*
            // the node on the opposite side of the portal
            BSP &front =
                (opposite ? *plane_parent->behind : *plane_parent->in_front)
                    .closest_node(center);
            */
            auto &b = this->root->get_point_node(
                center +
                    portals[i].shape.get_plane().normal * portal_width_epsilon,
                Matrix4x4::identity());
            b.merge_portal(portals[i]);
            portals.emplace_back(portals[i].shape, leaf, &b);
            /*
            std::cout << "leaf center = " << leaf->shape.get_center() << "\n";
            std::cout << "leaf min = (" << leaf->b_box.min << "), max = ("
                      << leaf->b_box.max << ")\n";
            std::cout << "front center = " << front.shape.get_center() << "\n";
            std::cout << "front min = (" << front.b_box.min << "), max = ("
                      << front.b_box.max << ")\n";
            std::cout << "b center = " << b.shape.get_center() << "\n";
            std::cout << "b min = (" << b.b_box.min << "), max = ("
                      << b.b_box.max << ")\n";
                      */
        }
    }
}

void BSPTree::remove_useless_portals()
{
    for (auto &leaf : this->leaves) {
        std::erase_if(leaf->leaf_info().portals,
                      [](const auto &portal) { return !portal.in_front; });
    }
}

void BSPTree::render(const MapEntity &parent, Frame &frame, const Camera &cam,
                     std::span<const Texture> texs)
{
    if (!render_innodes) {
        this->root->render(parent, frame, cam, texs);
        return;
    }

    this->root->reset_sort_keys();

    std::stack<BSP *> nodes;
    std::vector<const BSP *> visited;

    nodes.push(&this->root->get_point_node(cam.pos, parent));
    visited.push_back(nodes.top());

    // 6884

    isize_t key = 0;
    while (!nodes.empty()) {
        auto &cur = *nodes.top();
        nodes.pop();
        RenderPixels::render_point(cur.shape.get_center(), frame, cam,
                                   Color(255, 0, 0), 5, false);
        assert(cur.is_leaf());
        cur.render(parent, frame, cam, texs, key);

        for (auto portal = cur.leaf_info().portals.begin();
             portal < cur.leaf_info().portals.end(); ++portal) {
            assert(portal->in_front);
            BSP &other = *portal->in_front;
            assert(&other != &cur);
            if (std::find(visited.rbegin(), visited.rend(), &other) !=
                visited.rend()) {
                continue;
            }

            /*
            if (!poly_visible(portal->shape, parent.get_transform(), frame,
                              cam))
                continue;
                */

            nodes.push(&other);
            visited.push_back(&other);
        }

        ++key;
    }
}

isize_t BSPTree::leaf_idx(const BSP &leaf) const
{
    auto i = std::find(this->leaves.begin(), this->leaves.end(), &leaf);
    if (i == this->leaves.end())
        return -1;
    else
        return std::distance(this->leaves.begin(), i);
}
