#include "shape.hpp"
#include "constants.hpp"
#include "plane.hpp"
#include "polygon.hpp"
#include "ssize.hpp"
#include "vector/vec3.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

// TODO: remove all the duplicate code

namespace {

void erase_duplicate_points(std::vector<Vec3> &pts)
{
    for (auto i = pts.begin(); i < pts.end(); ++i) {
        for (auto j = pts.begin(); j < pts.end(); ++j) {
            if (i == j)
                continue;

            if (i->dist(*j) > Consts::epsilon)
                continue;

            pts.erase(j);
            if (i > j)
                --i;
            break;
        }
    }
}

void erase_duplicate_points(std::vector<TexVert> &pts)
{
    for (auto i = pts.begin(); i < pts.end(); ++i) {
        for (auto j = pts.begin(); j < pts.end(); ++j) {
            if (i == j)
                continue;

            if (i->v.dist(j->v) > Consts::epsilon)
                continue;

            pts.erase(j);
            if (i > j)
                --i;
            break;
        }
    }
}

} // namespace

// ================================================
// ConvexShape
// ================================================

ConvexShape::ConvexShape(std::span<const Polygon> polys)
{
    this->polys.assign(polys.begin(), polys.end());
}

ConvexShape ConvexShape::box(Vec3 scale)
{
    Vec3 s = scale / 2.f;

    std::vector<Polygon> polys;

    polys.push_back(
        Polygon(std::array{Vec3(-s.x, -s.y, -s.z), Vec3(s.x, -s.y, -s.z),
                           Vec3(s.x, s.y, -s.z), Vec3(-s.x, s.y, -s.z)}));
    polys.push_back(
        Polygon(std::array{Vec3(s.x, -s.y, s.z), Vec3(-s.x, -s.y, s.z),
                           Vec3(-s.x, s.y, s.z), Vec3(s.x, s.y, s.z)}));

    polys.push_back(
        Polygon(std::array{Vec3(-s.x, -s.y, s.z), Vec3(-s.x, -s.y, -s.z),
                           Vec3(-s.x, s.y, -s.z), Vec3(-s.x, s.y, s.z)}));
    polys.push_back(
        Polygon(std::array{Vec3(s.x, s.y, s.z), Vec3(s.x, s.y, -s.z),
                           Vec3(s.x, -s.y, -s.z), Vec3(s.x, -s.y, s.z)}));

    polys.push_back(
        Polygon(std::array{Vec3(-s.x, -s.y, -s.z), Vec3(-s.x, -s.y, s.z),
                           Vec3(s.x, -s.y, s.z), Vec3(s.x, -s.y, -s.z)}));
    polys.push_back(
        Polygon(std::array{Vec3(s.x, s.y, -s.z), Vec3(s.x, s.y, s.z),
                           Vec3(-s.x, s.y, s.z), Vec3(-s.x, s.y, -s.z)}));

    return ConvexShape(polys);
}

void ConvexShape::remove_empty_polys()
{
    std::erase_if(this->polys, [](const Polygon &poly) {
        return poly.vs.empty() || poly.get_area() < Consts::epsilon;
    });
}

void ConvexShape::clip(const Plane &plane)
{
    auto intersections = this->get_intersections(plane);

    for (auto &poly : this->polys) {
        poly.clip(plane);
    }
    this->remove_empty_polys();

    if (intersections.size() >= 3) {
        // creates a poly to fill in the hole in the shape caused by clipping
        this->polys.emplace_back(intersections, -plane.normal);
    }
}

std::vector<Vec3> ConvexShape::get_intersections(const Plane &plane) const
{
    std::vector<Vec3> pts;

    for (const auto &poly : this->polys) {
        auto poly_pts = poly.get_intersections(plane);
        pts.insert(pts.end(), poly_pts.begin(), poly_pts.end());
    }

    erase_duplicate_points(pts);

    return pts;
}

std::vector<Triangle> ConvexShape::get_triangles() const
{
    std::vector<Triangle> tris;

    for (const auto &poly : this->polys) {
        auto poly_tris = poly.get_triangles();
        tris.insert(tris.end(), poly_tris.begin(), poly_tris.end());
    }

    return tris;
}

AABB ConvexShape::get_aabb() const
{
    assert(!this->polys.empty());

    AABB box = AABB::map_box();
    std::swap(box.max, box.min);

    for (const auto &poly : this->polys) {
        for (const auto &v : poly.vs) {
            box.merge(v);
        }
    }

    return box;
}

bool ConvexShape::contains(const Vec3 &p, float epsilon) const
{
    for (const auto &poly : this->polys) {
        auto plane = poly.get_plane();
        if (plane.is_point_in_front(p) && !plane.is_point_on(p, epsilon)) {
            return false;
        }
    }

    return true;
}

Vec3 ConvexShape::get_center() const
{
    Vec3 center = Vec3::zero();
    isize_t n = 0;

    for (const auto &poly : this->polys) {
        for (const auto &v : poly.vs) {
            ++n;
            center += v;
        }
    }

    center /= n;
    return center;
}

// ================================================
// BrushShape
// ================================================

BrushShape::BrushShape(std::span<const RenderPolygon> polys)
{
    this->polys.assign(polys.begin(), polys.end());
}

void BrushShape::remove_empty_polys()
{
    std::erase_if(this->polys, [](const auto &poly) {
        return poly.vs.empty() || poly.get_area() < Consts::epsilon;
    });
}

void BrushShape::clip(const Plane &plane, isize_t new_tex_idx)
{
    if (new_tex_idx == -1)
        new_tex_idx = this->polys.front().tex_idx;

    auto intersections = this->get_intersections(plane);

    for (auto &poly : this->polys) {
        poly.clip(plane);
    }
    this->remove_empty_polys();

    if (intersections.size() >= 3) {
        // creates a poly to fill in the hole in the shape caused by clipping
        this->polys.emplace_back(intersections, new_tex_idx, -plane.normal);
    }
}

std::vector<TexVert> BrushShape::get_intersections(const Plane &plane) const
{
    std::vector<TexVert> pts;

    for (const auto &poly : this->polys) {
        auto poly_pts = poly.get_intersections(plane);
        pts.insert(pts.end(), poly_pts.begin(), poly_pts.end());
    }

    erase_duplicate_points(pts);

    return pts;
}

std::vector<Triangle> BrushShape::get_triangles() const
{
    std::vector<Triangle> tris;

    for (const auto &poly : this->polys) {
        auto poly_tris = poly.get_triangles();
        tris.insert(tris.end(), poly_tris.begin(), poly_tris.end());
    }

    return tris;
}

AABB BrushShape::get_aabb() const
{
    assert(!this->polys.empty());

    AABB box = AABB::map_box();
    std::swap(box.max, box.min);

    for (const auto &poly : this->polys) {
        for (const auto &v : poly.vs) {
            box.merge(v.v);
        }
    }

    return box;
}

bool BrushShape::contains(const Vec3 &p, float epsilon) const
{
    for (const auto &poly : this->polys) {
        auto plane = poly.get_plane();
        if (plane.is_point_in_front(p) && !plane.is_point_on(p, epsilon)) {
            return false;
        }
    }

    return true;
}

Vec3 BrushShape::get_center() const
{
    Vec3 center = Vec3::zero();
    isize_t n = 0;

    for (const auto &poly : this->polys) {
        for (const auto &v : poly.vs) {
            ++n;
            center += v.v;
        }
    }

    center /= n;
    return center;
}
