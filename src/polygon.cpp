#include "polygon.hpp"
#include "angle.hpp"
#include "plane.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <span>
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

Polygon::Polygon(std::span<const Vec3> vs, const Vec3 &normal)
{
    assert(vs.size() >= 3);
    this->vs.assign(vs.begin(), vs.end());
    this->sort_vs_ccw(normal);
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

void Polygon::sort_vs_ccw(Vec3 intended_normal)
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

        if (behind != prev_behind) {
            auto p = std::get<0>(plane.line_intersect_point(*prev, *v));
            std::cout << "intersection at (" << p << ")\n";
            intersections.push_back(
                std::get<0>(plane.line_intersect_point(*prev, *v)));
        }

        prev = v;
    }

    return intersections;
}

std::vector<Triangle> Polygon::get_triangles() const
{
    std::vector<Triangle> tris;

    for (size_t i = 0; i < this->vs.size(); ++i)
        std::cout << "v[" << i << "] = (" << this->vs[i] << ")\n";

    for (size_t i = 1; i < this->vs.size(); i += 2) {
        size_t next = (i + 1) % this->vs.size();
        tris.emplace_back(
            std::array{this->vs[i], this->vs[next], this->vs[i - 1]},
            std::array{Vec2(0.f, 0.f), Vec2(1.f, 0.f), Vec2(1.f, 1.f)}, 0);
    }

    return tris;
}
