#include "polygon.hpp"
#include "angle.hpp"
#include "plane.hpp"
#include <cassert>
#include <cstdint>
#include <iterator>
#include <span>
#include <utility>

Polygon::Polygon(std::span<const Vec3> vs)
{
    assert(vs.size() >= 3);
    this->vs.assign(vs.begin(), vs.end());
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

    for (auto v = this->vs.begin(); v < this->vs.end() - 2; ++v) {
        Vec3 a = (*v - center).normalize();

        Plane plane(std::array{*v, center, center + intended_normal});

        Angle smallest_angle(-1.f);
        size_t smallest_idx = SIZE_MAX;
        for (auto w = v + 1; w < this->vs.end(); ++w) {
            if (plane.is_point_behind(*w))
                continue;

            Vec3 b = (*w - center).normalize();

            Angle angle(a.dot(b));
            if (angle > smallest_angle) {
                smallest_angle = angle;
                smallest_idx = std::distance(this->vs.begin(), w);
            }
        }

        // if this fires, the polygon is degenerate
        assert(smallest_idx != SIZE_MAX);

        std::swap(this->vs[smallest_idx],
                  this->vs[std::distance(this->vs.begin(), v) + 1]);
    }
}
