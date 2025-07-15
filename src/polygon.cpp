#include "polygon.hpp"
#include "plane.hpp"
#include <cassert>
#include <iterator>

namespace {

void remove_duplicates(std::vector<Vec3> &points)
{
    for (size_t i = 0; i < points.size(); ++i) {
        for (size_t j = 0; j < points.size(); ++j) {
            if (i == j || points[i].dist(points[j]) > 0.001f)
                continue;

            points.erase(points.begin() + j--);
            if (i > j)
                --i;
        }
    }
}

} // namespace

Polygon::Polygon(std::span<const Vec3> vs)
{
    this->vs.assign(std::begin(vs), std::end(vs));
}

Plane Polygon::get_plane() const
{
    assert(this->vs.size() >= 2);

    Vec3 a = this->vs[1] - this->vs[0];
    Vec3 b = this->vs[2] - this->vs[0];

    Vec3 normal = b.cross(a).normalize();
    float d = normal.dot(this->vs[0]);

    return Plane(normal, d);
}

void Polygon::clip(const Plane &plane)
{
    std::vector<Vec3> new_vs;
    Vec3 prev_v = this->vs.back();

    for (const auto &v : this->vs) {
        bool v_behind = plane.is_point_behind(v);
        bool prev_behind = plane.is_point_behind(prev_v);

        if (prev_behind != v_behind) {
            new_vs.push_back(
                std::get<0>(plane.line_intersect_point(prev_v, v)));
            // std::cout << "prev v = {" << prev_v << "}\n";
            // std::cout << "v = {" << v << "}\n";
        }

        if (!v_behind)
            new_vs.push_back(v);

        prev_v = v;
    }

    remove_duplicates(new_vs);
    this->vs = std::move(new_vs);
}

std::vector<Vec3> Polygon::plane_intersections(const Plane &plane) const
{
    std::vector<Vec3> points;

    Vec3 prev_v = this->vs.back();

    for (const auto &v : this->vs) {
        bool v_behind = plane.is_point_behind(v);
        bool prev_behind = plane.is_point_behind(prev_v);

        if (prev_behind != v_behind)
            points.push_back(
                std::get<0>(plane.line_intersect_point(prev_v, v)));

        prev_v = v;
    }

    return points;
}
