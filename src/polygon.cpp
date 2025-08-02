#include "polygon.hpp"
#include "angle.hpp"
#include "mat4x4.hpp"
#include "plane.hpp"
#include "transform.hpp"
#include "vector/vec2.hpp"
#include "vector/vec3.hpp"
#include "vertex.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

// TODO: remove all the duplicate code

namespace {

Angle signed_angle_between(const Vec3 &v, const Vec3 &w, const Vec3 &normal)
{
    float dot = v.dot(w);
    float det = normal.dot(v.cross(w));
    return Angle(std::atan2(det, dot));
}

} // namespace

// ========================================================
// Polygon
// ========================================================

Polygon::Polygon(std::span<const Vec3> vs)
{
    assert(vs.size() >= 3);
    this->vs.assign(vs.begin(), vs.end());
}

Polygon::Polygon(std::span<const Vec3> vs, const Vec3 &normal) : Polygon(vs)
{
    this->sort_vs_ccw(normal);
}

Polygon::Polygon(const RenderPolygon &other)
{
    for (const auto &v : other.vs) {
        this->vs.push_back(v.v);
    }
}

Polygon::Polygon(const Triangle &tri)
    : Polygon(std::array{tri.vs[0], tri.vs[1], tri.vs[2]})
{}

Vec3 Polygon::get_center() const
{
    Vec3 c = Vec3::zero();

    for (const auto &v : this->vs) {
        c += v;
    }

    c /= this->vs.size();
    return c;
}

// algorithm from https://stackoverflow.com/a/32275005
// modified to work with ccw winding order
Plane Polygon::get_plane() const
{
    Vec3 c = this->get_center();

    Vec3 sum = Vec3::zero();

    for (size_t i = 0; i < this->vs.size(); ++i) {
        size_t next = (i + 1) % this->vs.size();

        sum += (this->vs[next] - c).cross(this->vs[i] - c);
    }

    Vec3 n = sum.normalize();
    float d = n.dot(this->vs[0]);
    return Plane(n, d);
}

void Polygon::sort_vs_ccw(const Vec3 &intended_normal)
{
    Vec3 center = this->get_center();

    Vec3 angle_basis = this->vs[0] - center;

    std::sort(
        this->vs.begin(), this->vs.end(),
        [center, intended_normal, angle_basis](const Vec3 &a, const Vec3 &b) {
            Angle a_angle =
                signed_angle_between(a - center, angle_basis, intended_normal);
            Angle b_angle =
                signed_angle_between(b - center, angle_basis, intended_normal);
            return a_angle < b_angle;
        });
}

void Polygon::clip(const Plane &plane)
{
    std::vector<Vec3> new_vs;

    auto prev = this->vs.end() - 1;
    for (auto v = this->vs.begin(); v < this->vs.end(); ++v) {
        bool behind = plane.is_point_behind(*v);
        bool prev_behind = plane.is_point_behind(*prev);

        if (behind != prev_behind)
            new_vs.push_back(
                std::get<0>(plane.line_intersect_point(*prev, *v)));

        if (!behind) {
            new_vs.push_back(*v);
        }

        prev = v;
    }

    this->vs = std::move(new_vs);
}

std::vector<Vec3> Polygon::get_intersections(const Plane &plane) const
{
    std::vector<Vec3> intersections;

    auto prev = this->vs.end() - 1;
    for (auto v = this->vs.begin(); v < this->vs.end(); ++v) {
        bool behind = plane.is_point_behind(*v);
        bool prev_behind = plane.is_point_behind(*prev);

        if (behind != prev_behind)
            intersections.push_back(
                std::get<0>(plane.line_intersect_point(*prev, *v)));

        prev = v;
    }

    return intersections;
}

std::vector<Triangle> Polygon::get_triangles(bool cull_degenerates) const
{
    assert(this->vs.size() >= 3);

    std::vector<Triangle> tris;

    size_t anchor = 0;
    for (size_t i = 1; i < this->vs.size() - 1; ++i) {
        Triangle new_tri(
            std::array{this->vs[anchor], this->vs[i], this->vs[i + 1]},
            std::array{Vec2(0.f, 0.f), Vec2(1.f, 0.f), Vec2(1.f, 1.f)}, 0);

        if (cull_degenerates && new_tri.get_area() <= 0.01f)
            continue;
        tris.push_back(new_tri);
    }

    return tris;
}

AABB Polygon::get_aabb() const
{
    assert(!this->vs.empty());

    Vec3 min = this->vs[0];
    Vec3 max = this->vs[0];

    for (const auto &v : this->vs) {
        min.x = std::min(min.x, v.x);
        min.y = std::min(min.y, v.y);
        min.z = std::min(min.z, v.z);

        max.x = std::min(max.x, v.x);
        max.y = std::min(max.y, v.y);
        max.z = std::min(max.z, v.z);
    }

    return AABB(min, max);
}

bool Polygon::empty() const
{
    return this->vs.empty();
}

bool Polygon::invalid() const
{
    return this->vs.size() < 3;
}

bool Polygon::is_on(const Plane &plane, float epsilon) const
{
    for (const auto &v : this->vs) {
        if (!plane.is_point_on(v, epsilon))
            return false;
    }

    return true;
}

float Polygon::get_area() const
{
    auto tris = this->get_triangles();

    float area = 0.f;
    for (const auto &tri : tris) {
        area += tri.get_area();
    }

    return area;
}

std::vector<Plane> Polygon::side_planes() const
{
    std::vector<Plane> planes;

    for (auto p = this->vs.begin(); p < this->vs.end(); ++p) {
        // p and next form a line
        auto next = p == this->vs.end() - 1 ? this->vs.begin() : p + 1;

        Vec3 center = p->mix(*next, 0.5f);
        Vec3 normal = (center - this->get_center()).normalize();
        planes.emplace_back(normal, *p);
    }

    return planes;
}

bool Polygon::partially_contains(const Polygon &other, float epsilon) const
{
    if (!this->is_on(other.get_plane(), epsilon))
        return false;

    // seperating axis theorem

    for (const auto &plane : this->side_planes()) {
        if (other.is_in_front(plane))
            return false;
    }

    for (const auto &plane : other.side_planes()) {
        if (this->is_in_front(plane))
            return false;
    }

    return true;
}

bool Polygon::partially_contains(const Triangle &tri, float epsilon) const
{
    return this->partially_contains(
        Polygon(std::array{tri.vs[0], tri.vs[1], tri.vs[2]}), epsilon);
}

bool Polygon::is_in_front(const Plane &plane) const
{
    for (const auto &v : this->vs) {
        if (!plane.is_point_in_front(v))
            return false;
    }

    return true;
}

bool Polygon::is_behind(const Plane &plane) const
{
    for (const auto &v : this->vs) {
        if (!plane.is_point_behind(v))
            return false;
    }

    return true;
}

void Polygon::flip_dir()
{
    std::reverse(this->vs.begin(), this->vs.end());
}

std::ostream &operator<<(std::ostream &os, const Polygon &poly)
{
    for (auto v = poly.vs.begin(); v < poly.vs.end(); ++v) {
        if (v > poly.vs.begin())
            os << ", ";
        os << "(" << *v << ")";
    }

    return os;
}

// ========================================================
// RenderPolygon
// ========================================================

RenderPolygon::RenderPolygon(std::span<const Vec3> vs,
                             std::span<const Vec2> vts, size_t tex_idx)
{
    assert(vs.size() >= 3);
    assert(vts.size() == vs.size());

    this->tex_idx = tex_idx;

    for (size_t i = 0; i < vs.size(); ++i) {
        this->vs.emplace_back(vs[i], vts[i]);
    }
}

RenderPolygon::RenderPolygon(std::span<const TexVert> vs, size_t tex_idx)
{
    assert(vs.size() >= 3);

    this->tex_idx = tex_idx;

    for (size_t i = 0; i < vs.size(); ++i) {
        this->vs.push_back(vs[i]);
    }
}

RenderPolygon::RenderPolygon(std::span<const Vec3> vs,
                             std::span<const Vec2> vts, size_t tex_idx,
                             const Vec3 &normal)
    : RenderPolygon(vs, vts, tex_idx)
{
    this->sort_vs_ccw(normal);
}

RenderPolygon::RenderPolygon(std::span<const TexVert> vs, size_t tex_idx,
                             const Vec3 &normal)
    : RenderPolygon(vs, tex_idx)
{
    this->sort_vs_ccw(normal);
}

RenderPolygon::RenderPolygon(const Triangle &tri)
    : RenderPolygon(tri.vs, tri.vts, tri.tex_idx)
{}

Vec3 RenderPolygon::get_center() const
{
    Vec3 c = Vec3::zero();

    for (const auto &v : this->vs) {
        c += v.v;
    }

    c /= this->vs.size();
    return c;
}

Plane RenderPolygon::get_plane() const
{
    Vec3 c = this->get_center();

    Vec3 sum = Vec3::zero();

    for (size_t i = 0; i < this->vs.size(); ++i) {
        size_t next = (i + 1) % this->vs.size();

        sum += (this->vs[next].v - c).cross(this->vs[i].v - c);
    }

    Vec3 n = sum.normalize();
    float d = n.dot(this->vs[0].v);
    return Plane(n, d);
}

void RenderPolygon::sort_vs_ccw(const Vec3 &intended_normal)
{
    Vec3 center = this->get_center();
    Vec3 angle_basis = this->vs[0].v - center;

    std::sort(this->vs.begin(), this->vs.end(),
              [center, intended_normal, angle_basis](const TexVert &a,
                                                     const TexVert &b) {
                  Angle a_angle = signed_angle_between(
                      a.v - center, angle_basis, intended_normal);
                  Angle b_angle = signed_angle_between(
                      b.v - center, angle_basis, intended_normal);
                  return a_angle < b_angle;
              });
}

void RenderPolygon::clip(const Plane &plane)
{
    std::vector<TexVert> new_vs;

    auto prev = this->vs.end() - 1;
    for (auto v = this->vs.begin(); v < this->vs.end(); ++v) {
        bool behind = plane.is_point_behind(v->v);
        bool prev_behind = plane.is_point_behind(prev->v);

        if (behind != prev_behind) {
            Vec3 p;
            float t;
            std::tie(p, t) = plane.line_intersect_point(prev->v, v->v);
            new_vs.emplace_back(p, prev->vt.mix(v->vt, t));
        }

        if (!behind) {
            new_vs.push_back(*v);
        }

        prev = v;
    }

    this->vs = std::move(new_vs);
}

std::vector<TexVert> RenderPolygon::get_intersections(const Plane &plane) const
{
    std::vector<TexVert> intersections;

    auto prev = this->vs.end() - 1;
    for (auto v = this->vs.begin(); v < this->vs.end(); ++v) {
        bool behind = plane.is_point_behind(v->v);
        bool prev_behind = plane.is_point_behind(prev->v);

        if (behind != prev_behind) {
            Vec3 p;
            float t;
            std::tie(p, t) = plane.line_intersect_point(prev->v, v->v);
            Vec2 p_tex = prev->vt.mix(v->vt, t);
            intersections.emplace_back(p, p_tex);
        }

        prev = v;
    }

    return intersections;
}

std::vector<Vec2i> RenderPolygon::get_screen_vs(const Matrix4x4 &transform,
                                                const Camera &cam) const
{
    std::vector<Vec2i> screen_vs;
    screen_vs.reserve(this->vs.size());

    for (const auto &v : this->vs) {
        Vec3 transf_v = transform * Vec4(v.v, 1.f);
        Vec3 cam_space = Transform::world_space_to_cam_space(transf_v, cam);
        Vec2 norm_scr = Transform::cam_space_to_norm_scr(cam_space, cam);
        Vec2i scr_space = Transform::norm_scr_to_scr_space(norm_scr);
        screen_vs.push_back(scr_space);
    }

    return screen_vs;
}

std::vector<Vec3> RenderPolygon::get_cam_space_vs(const Matrix4x4 &transform,
                                                  const Camera &cam) const
{
    std::vector<Vec3> ret;

    for (const auto &v : this->vs) {
        Vec3 transf_v = transform * Vec4(v.v, 1.f);
        Vec3 cam_space = Transform::world_space_to_cam_space(transf_v, cam);
        ret.push_back(cam_space);
    }

    return ret;
}

std::vector<Triangle> RenderPolygon::get_triangles(bool cull_degenerates) const
{
    assert(this->vs.size() >= 3);

    std::vector<Triangle> tris;

    size_t anchor = 0;
    for (size_t i = 1; i < this->vs.size() - 1; ++i) {
        Triangle new_tri(
            std::array{this->vs[anchor].v, this->vs[i].v, this->vs[i + 1].v},
            std::array{this->vs[anchor].vt, this->vs[i].vt, this->vs[i + 1].vt},
            this->tex_idx);

        if (cull_degenerates && new_tri.get_area() <= 0.01f)
            continue;
        tris.push_back(new_tri);
    }

    return tris;
}

bool RenderPolygon::empty() const
{
    return this->vs.empty();
}

bool RenderPolygon::invalid() const
{
    return this->vs.size() < 3;
}

float RenderPolygon::get_area() const
{
    auto tris = this->get_triangles();

    float area = 0.f;
    for (const auto &tri : tris) {
        area += tri.get_area();
    }

    return area;
}

bool RenderPolygon::intersects(const Plane &plane) const
{
    assert(this->vs.size() >= 3);

    bool v0_b = plane.is_point_behind(this->vs[0].v);
    for (auto v = this->vs.begin() + 1; v < this->vs.end(); ++v) {
        if (plane.is_point_behind(v->v) != v0_b)
            return true;
    }

    return false;
}

bool RenderPolygon::partially_contains(const Polygon &other,
                                       float epsilon) const
{
    if (!this->is_on(other.get_plane(), epsilon))
        return false;

    // seperating axis theorem

    for (const auto &plane : this->side_planes()) {
        if (other.is_in_front(plane))
            return false;
    }

    for (const auto &plane : other.side_planes()) {
        if (this->is_in_front(plane))
            return false;
    }

    return true;
}

bool RenderPolygon::partially_contains(const Triangle &tri, float epsilon) const
{
    return this->partially_contains(
        Polygon(std::array{tri.vs[0], tri.vs[1], tri.vs[2]}), epsilon);
}

bool RenderPolygon::is_in_front(const Plane &plane) const
{
    for (const auto &v : this->vs) {
        if (!plane.is_point_in_front(v.v))
            return false;
    }

    return true;
}

bool RenderPolygon::is_behind(const Plane &plane) const
{
    for (const auto &v : this->vs) {
        if (!plane.is_point_behind(v.v))
            return false;
    }

    return true;
}

std::vector<Plane> RenderPolygon::side_planes() const
{
    std::vector<Plane> planes;

    for (auto p = this->vs.begin(); p < this->vs.end(); ++p) {
        // p and next form a line
        auto next = p == this->vs.end() - 1 ? this->vs.begin() : p + 1;

        Vec3 center = p->v.mix(next->v, 0.5f);
        Vec3 normal = (center - this->get_center()).normalize();
        planes.emplace_back(normal, p->v);
    }

    return planes;
}

std::ostream &operator<<(std::ostream &os, const RenderPolygon &poly)
{
    for (auto v = poly.vs.begin(); v < poly.vs.end(); ++v) {
        if (v > poly.vs.begin())
            os << ", ";
        os << "((" << v->v << "), (" << v->vt << "))";
    }

    return os;
}
