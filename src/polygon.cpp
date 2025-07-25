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

namespace {

Angle signed_angle_between(const Vec3 &v, const Vec3 &w, const Vec3 &normal)
{
    float dot = v.dot(w);
    float det = normal.dot(v.cross(w));
    return Angle(std::atan2(det, dot));
}

} // namespace

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

std::vector<Triangle> Polygon::get_triangles() const
{
    std::vector<Triangle> tris;

    size_t anchor = 0;
    for (size_t i = 1; i < this->vs.size() - 1; ++i) {
        tris.emplace_back(
            std::array{this->vs[anchor], this->vs[i], this->vs[i + 1]},
            std::array{Vec2(0.f, 0.f), Vec2(1.f, 0.f), Vec2(1.f, 1.f)}, 0);
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

RenderPolygon::RenderPolygon(std::span<const Vec3> vs,
                             std::span<const Vec2> vts, size_t tex_idx,
                             const Vec3 &normal)
    : RenderPolygon(vs, vts, tex_idx)
{
    this->sort_vs_ccw(normal);
}

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

std::vector<Triangle> RenderPolygon::get_triangles() const
{
    assert(this->vs.size() >= 3);

    std::vector<Triangle> tris;

    size_t anchor = 0;
    for (size_t i = 1; i < this->vs.size() - 1; ++i) {
        tris.emplace_back(
            std::array{this->vs[anchor].v, this->vs[i].v, this->vs[i + 1].v},
            std::array{this->vs[anchor].vt, this->vs[i].vt, this->vs[i + 1].vt},
            this->tex_idx);
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
    // TODO: bruh improve this

    auto tris = this->get_triangles();

    float area = 0.f;
    for (const auto &tri : tris) {
        area += tri.get_area();
    }

    return area;
}
