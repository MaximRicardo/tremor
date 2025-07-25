#include "shape.hpp"
#include "constants.hpp"
#include "plane.hpp"
#include "polygon.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

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

} // namespace

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
    std::erase_if(this->polys,
                  [](const Polygon &poly) { return poly.vs.empty(); });
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
        Polygon new_poly(intersections, -plane.normal);
        this->polys.push_back(new_poly);
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
