#include "pvs.hpp"
#include "bsp.hpp"
#include "constants.hpp"
#include <cassert>
#include <optional>
#include <vector>

namespace {

std::vector<Plane> get_seperators(const Polygon &start, const Polygon &pass)
{
    std::vector<Plane> planes;

    for (auto e_start = start.vs.begin(); e_start < start.vs.end(); ++e_start) {
        auto e_end =
            e_start == start.vs.end() - 1 ? start.vs.begin() : e_start + 1;
        Vec3 e_center = e_start->mix(*e_end, 0.5f);
        Vec3 e_dir = (*e_end - *e_start).normalize();

        for (const auto &v : pass.vs) {
            Vec3 perp_normal = (v - e_center).normalize();
            Vec3 normal = e_dir.cross(perp_normal);
            if (normal.length() < Consts::epsilon)
                continue;
            Plane p(normal, v);

            // every vertex in self should be behind p, while every vertex in
            // pass should be in front of p. if not, this plane isn't a proper
            // seperator and can be discarded.
            bool this_behind = start.is_behind(p) || start.is_on(p);
            bool pass_in_front = pass.is_in_front_of(p) || pass.is_on(p);
            if (!this_behind || !pass_in_front)
                continue;

            planes.push_back(p);
        }
    }

    // std::cout << "n planes = " << planes.size() << "\n";
    // assert(!planes.empty());
    return planes;
}

} // namespace

PVS::Portal::Portal(const Polygon &shape, BSP *behind, BSP *in_front)
    : shape(shape), behind(behind), in_front(in_front)
{}

void PVS::Portal::merge(Portal &a, Portal &b)
{
    assert(a.behind->is_leaf());
    assert(b.behind->is_leaf());

    a.in_front = b.behind;
    b.in_front = a.behind;

    bool a_smallest = a.shape.get_area() < b.shape.get_area();
    const Portal &smallest = a_smallest ? a : b;
    Portal &largest = a_smallest ? b : a;

    largest.shape = smallest.shape;
    largest.shape.flip_dir();
}

void PVS::Portal::merge_with(const Portal &other)
{
    assert(this->behind->is_leaf());
    assert(other.behind->is_leaf());

    this->in_front = other.behind;

    bool other_smallest = other.shape.get_area() < this->shape.get_area();
    if (other_smallest) {
        this->shape = other.shape;
        this->shape.flip_dir();
    }
}

std::optional<Polygon> PVS::Portal::is_visible(const Polygon &start,
                                               const Portal &end,
                                               const Polygon &pass)
{
    if (end.shape.get_plane().normal.dot(start.get_plane().normal) <
        -1.f + Consts::epsilon)
        return {};

    auto seps = get_seperators(start, pass);
    auto other_seps = get_seperators(pass, start);
    seps.insert(seps.end(), other_seps.begin(), other_seps.end());
    if (seps.empty())
        return {};

    Polygon end_poly = end.shape;

    for (const auto &p : seps) {
        end_poly.clip(p);
        if (end_poly.empty() || end_poly.get_area() < Consts::epsilon) {
            // std::cout << "got here\n";
            return {};
        }
        // std::cout << "area = " << end_poly.get_area() << "\n";
    }

    return end_poly;
}
