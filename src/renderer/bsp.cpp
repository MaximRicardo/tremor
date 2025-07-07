#include "bsp.hpp"
#include <cstddef>
#include <memory>

namespace {

constexpr float split_plane_epsilon = 0.0001f;

}

BSP::BSP(Triangle node_tri, BSP *parent)
    : node_tri(node_tri), node_plane(node_tri.get_plane()), parent(parent)
{}

BSP::BSP(const std::vector<Triangle> &tris, BSP *parent)
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

    if (tris_behind.n_tris > 0) {
        printf("normal before split = (%f, %f, %f)\n", tri.get_plane().normal.x,
               tri.get_plane().normal.y, tri.get_plane().normal.z);
        for (unsigned i = 0; i < tris_behind.n_tris; ++i) {
            printf("normal after behind split = (%f, %f, %f)\n",
                   tris_behind.tris[i].get_plane().normal.x,
                   tris_behind.tris[i].get_plane().normal.y,
                   tris_behind.tris[i].get_plane().normal.z);
        }
        printf("n = %u\n", tris_behind.n_tris);
    }

    if (tris_behind.n_tris == 0)
        return;

    size_t start_idx = 0;
    if (!this->lhs) {
        this->lhs = std::make_unique<BSP>(tris_behind.tris[0], this);
        start_idx = 1;
    }

    for (size_t i = start_idx; i < tris_behind.n_tris; ++i) {
        this->lhs->insert(tris_behind.tris[i]);
    }
}

void BSP::insert_tris_in_front(const Triangle &tri)
{
    auto tris_in_front =
        Plane(this->node_plane.normal, this->node_plane.d - split_plane_epsilon)
            .clip(tri);

    if (tris_in_front.n_tris > 0) {
        printf("normal before split = (%f, %f, %f)\n", tri.get_plane().normal.x,
               tri.get_plane().normal.y, tri.get_plane().normal.z);
        for (unsigned i = 0; i < tris_in_front.n_tris; ++i) {
            printf("normal after in front split = (%f, %f, %f)\n",
                   tris_in_front.tris[i].get_plane().normal.x,
                   tris_in_front.tris[i].get_plane().normal.y,
                   tris_in_front.tris[i].get_plane().normal.z);
        }
    }

    if (tris_in_front.n_tris == 0)
        return;

    size_t start_idx = 0;
    if (!this->rhs) {
        this->rhs = std::make_unique<BSP>(tris_in_front.tris[0], this);
        start_idx = 1;
    }

    for (size_t i = start_idx; i < tris_in_front.n_tris; ++i) {
        this->rhs->insert(tris_in_front.tris[i]);
    }
}

void BSP::insert(const Triangle &tri)
{
    if (this->node_plane.is_coplanar(tri.get_plane())) {
        if (!this->lhs)
            this->lhs = std::make_unique<BSP>(tri, this);
        else
            this->lhs->insert(tri);
    } else {
        this->insert_tris_behind(tri);
        this->insert_tris_in_front(tri);
    }
}

void BSP::insert(const std::vector<Triangle> &tris)
{
    for (auto &tri : tris) {
        this->insert(tri);
    }
}

void BSP::render(Color *frame, float *depth_buffer, const Camera &cam,
                 const Texture *texs) const
{
    bool cam_in_front =
        this->node_plane.normal.dot(cam.pos) >= this->node_plane.d;

    // the order the nodes have to be rendered in for complete back-to-front
    // rendering depends on whether the camera is in front of the current node
    // or not.
    auto &first_node = cam_in_front ? this->lhs : this->rhs;
    auto &last_node = cam_in_front ? this->rhs : this->lhs;

    if (first_node)
        first_node->render(frame, depth_buffer, cam, texs);

    if (cam_in_front)
        this->node_tri.render(frame, depth_buffer, cam, texs);

    if (last_node)
        last_node->render(frame, depth_buffer, cam, texs);
}

size_t BSP::n_triangles() const
{
    // starts at 1 to include this node's triangle
    size_t count = 1;

    if (this->lhs)
        count += this->lhs->n_triangles();
    if (this->rhs)
        count += this->rhs->n_triangles();

    return count;
}
