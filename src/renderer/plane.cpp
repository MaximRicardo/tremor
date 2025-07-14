#include "plane.hpp"
#include "triangle.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <tuple>

Plane::Plane(Vec3 normal, float d) : normal(normal), d(d) {}

namespace {

// matches the winding order of tri to that of other
// this is needed cuz for some strange reason that i can't figure out, when
// splitting a triangle the winding order sometimes changes and sometimes
// doesn't. my computer has something against me i swear
void match_winding_order(Triangle &tri, const Triangle &other)
{
    Plane tri_plane = tri.get_plane();
    Plane other_plane = other.get_plane();

    if (other_plane.normal.dot(tri_plane.normal) < 0.f) {
        std::swap(tri.vs[1], tri.vs[2]);
        std::swap(tri.vts[1], tri.vts[2]);
    }
}

// if only one vertex of the original triangle is in front of the splitting
// plane, one new triangle needs to be made. this function creates that
// triangle.
Triangle split_tri_1_in_front(const Plane &plane, const Triangle &tri,
                              size_t v_in_front, size_t v_behind_0,
                              size_t v_behind_1)
{
    Triangle sub_tri({}, {}, tri.tex_idx);

    float t;

    sub_tri.vs[0] = tri.vs[v_in_front];
    sub_tri.vts[0] = tri.vts[v_in_front];

    std::tie(sub_tri.vs[1], t) =
        plane.line_intersect_point(tri.vs[v_in_front], tri.vs[v_behind_0]);
    sub_tri.vts[1] = tri.vts[v_in_front].mix(tri.vts[v_behind_0], t);

    std::tie(sub_tri.vs[2], t) =
        plane.line_intersect_point(tri.vs[v_in_front], tri.vs[v_behind_1]);
    sub_tri.vts[2] = tri.vts[v_in_front].mix(tri.vts[v_behind_1], t);

    match_winding_order(sub_tri, tri);

    return sub_tri;
}

// if two vertices of the original triangle are in front of the splitting
// plane, two new triangles need to be made. this function creates the first
// triangle.
Triangle split_tri_2_in_front_0(const Plane &plane, const Triangle &tri,
                                size_t v_in_front_0, size_t v_in_front_1,
                                size_t v_behind)
{
    Triangle sub_tri({}, {}, tri.tex_idx);

    float t;

    sub_tri.vs[0] = tri.vs[v_in_front_0];
    sub_tri.vts[0] = tri.vts[v_in_front_0];

    std::tie(sub_tri.vs[1], t) =
        plane.line_intersect_point(tri.vs[v_in_front_0], tri.vs[v_behind]);
    sub_tri.vts[1] = tri.vts[v_in_front_0].mix(tri.vts[v_behind], t);

    sub_tri.vs[2] = tri.vs[v_in_front_1];
    sub_tri.vts[2] = tri.vts[v_in_front_1];

    match_winding_order(sub_tri, tri);

    return sub_tri;
}

// if two vertices of the original triangle are in front of the splitting
// plane, two new triangles need to be made. this function creates the second
// triangle.
Triangle split_tri_2_in_front_1(const Plane &plane, const Triangle &tri,
                                size_t v_in_front_0, size_t v_in_front_1,
                                size_t v_behind)
{
    Triangle sub_tri({}, {}, tri.tex_idx);

    float t;

    sub_tri.vs[0] = tri.vs[v_in_front_1];
    sub_tri.vts[0] = tri.vts[v_in_front_1];

    std::tie(sub_tri.vs[1], t) =
        plane.line_intersect_point(tri.vs[v_in_front_0], tri.vs[v_behind]);
    sub_tri.vts[1] = tri.vts[v_in_front_0].mix(tri.vts[v_behind], t);

    std::tie(sub_tri.vs[2], t) =
        plane.line_intersect_point(tri.vs[v_in_front_1], tri.vs[v_behind]);
    sub_tri.vts[2] = tri.vts[v_in_front_1].mix(tri.vts[v_behind], t);

    match_winding_order(sub_tri, tri);

    return sub_tri;
}

std::array<Triangle, 2>
split_tri_with_plane(const Plane &plane, const Triangle &tri,
                     const std::array<size_t, 3> &vs_in_front,
                     unsigned n_in_front,
                     const std::array<size_t, 3> &vs_behind, unsigned n_behind,
                     unsigned &n_sub_tris)
{
    assert(n_in_front + n_behind == 3);

    if (n_in_front == 0) {
        n_sub_tris = 0;
        return {Triangle({}, {}, tri.tex_idx), Triangle({}, {}, tri.tex_idx)};
    } else if (n_in_front == 1) {
        n_sub_tris = 1;
        return {split_tri_1_in_front(plane, tri, vs_in_front[0], vs_behind[0],
                                     vs_behind[1]),
                Triangle({}, {}, tri.tex_idx)};
    } else if (n_in_front == 2) {
        n_sub_tris = 2;
        return {split_tri_2_in_front_0(plane, tri, vs_in_front[0],
                                       vs_in_front[1], vs_behind[0]),
                split_tri_2_in_front_1(plane, tri, vs_in_front[0],
                                       vs_in_front[1], vs_behind[0])};
    } else {
        n_sub_tris = 1;
        return {tri, Triangle({}, {}, tri.tex_idx)};
    }
}

} // namespace

std::tuple<Vec3, float> Plane::line_intersect_point(const Vec3 &start,
                                                    const Vec3 &end) const
{
    float ad = start.dot(this->normal);
    float bd = end.dot(this->normal);
    float t = (this->d - ad) / (bd - ad);
    Vec3 line_start_to_end = end - start;
    Vec3 line_to_intersect = line_start_to_end * t;
    return std::make_tuple(start + line_to_intersect, t);
}

Plane::ClipTriangleRet Plane::clip(const Triangle &tri) const
{
    std::array<size_t, 3> vs_in_front;
    unsigned n_in_front = 0;
    std::array<size_t, 3> vs_behind;
    unsigned n_behind = 0;

    for (size_t i = 0; i < tri.vs.size(); i++) {
        float v_d = normal.dot(tri.vs[i]);

        if (v_d < this->d)
            vs_behind[n_behind++] = i;
        else
            vs_in_front[n_in_front++] = i;
    }

    struct Plane::ClipTriangleRet ret;
    ret.tris = split_tri_with_plane(*this, tri, vs_in_front, n_in_front,
                                    vs_behind, n_behind, ret.n_tris);

    return ret;
}

bool Plane::is_coplanar(const Plane &plane) const
{
    using namespace std;
    return abs(abs(this->normal.x) - abs(plane.normal.x)) < 0.01f &&
           abs(abs(this->normal.y) - abs(plane.normal.y)) < 0.01f &&
           abs(abs(this->normal.y) - abs(plane.normal.y)) < 0.01f &&
           abs(abs(this->d) - abs(plane.d)) < 0.01f;
}
