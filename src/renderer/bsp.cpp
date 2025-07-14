#include "bsp.hpp"
#include <cstddef>
#include <memory>

namespace {

constexpr float split_plane_epsilon = 0.0001f;

}

BSP::BSP(Triangle node_tri, BSP *parent)
    : node_tri(node_tri), node_plane(node_tri.get_plane()), parent(parent)
{}

BSP::BSP(std::span<const Triangle> tris, BSP *parent)
    : node_tri({}, {}), node_plane(Vec3(), 0.f), parent(parent)
{
    if (tris.size() == 0)
        return;

    this->node_tri = tris[0];
    this->node_plane = tris[0].get_plane();

    std::vector<Triangle> other_tris;
    other_tris.reserve(tris.size() - 1);
    for (size_t i = 1; i < tris.size(); ++i) {
        other_tris.push_back(tris[i]);
    }

    this->insert(other_tris);
}

void BSP::insert_tris_behind(const Triangle &tri)
{
    auto tris_behind = Plane(-this->node_plane.normal,
                             -this->node_plane.d - split_plane_epsilon)
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
    auto tris_in_front =
        Plane(this->node_plane.normal, this->node_plane.d - split_plane_epsilon)
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
    if (this->node_plane.is_coplanar(tri.get_plane())) {
        if (!this->behind)
            this->behind = std::make_unique<BSP>(tri, this);
        else
            this->behind->insert(tri);
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
    bool cam_in_front =
        this->node_plane.normal.dot(cam.pos) >= this->node_plane.d;

    // since we wanna render everything back to front, we gotta flip around
    // behind and in front depending on whether the camera's definition of in
    // front agrees with the current node or not.
    auto &first = cam_in_front ? this->behind : this->in_front;
    auto &last = cam_in_front ? this->in_front : this->behind;

    if (first)
        first->render(frame, depth_buffer, cam, texs);

    if (cam_in_front)
        this->node_tri.render(frame, depth_buffer, cam, texs);

    if (last)
        last->render(frame, depth_buffer, cam, texs);
}

size_t BSP::n_triangles() const
{
    // starts at 1 to include this node's triangle
    size_t count = 1;

    if (this->behind)
        count += this->behind->n_triangles();
    if (this->in_front)
        count += this->in_front->n_triangles();

    return count;
}
