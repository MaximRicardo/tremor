#include "hull.hpp"
#include "mat3x3.hpp"
#include "vector/vec3.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iterator>
#include <numeric>
#include <vector>

namespace {

// ASSUMES V AND W LIE ON A PLANE WITH THE GIVEN NORMAL VECTOR
// the formula is from here:
// https://stackoverflow.com/questions/14066933/direct-way-of-computing-the-clockwise-angle-between-two-vectors,
// in the accepted answer, under the "Plane embedded in 3D" section.
// i swear vro, stack overflow and mathematics stack exchange are my saviours.
Angle signed_angle_between(const Vec3 &v, const Vec3 &w, const Vec3 &normal)
{
    float dot = v.dot(w);
    float det = normal.dot(v.cross(w));
    return Angle(std::atan2(det, dot));
}

/*
Vec3 transform_z_axis(const Vec3 &point, const Vec3 &new_z)
{
    Matrix3x3 basis_mat(std::array<std::array<float, 3>, 3>{
        std::array{1.f, 0.f, 0.f}, std::array{0.f, 1.f, 0.f},
        std::array{new_z.x, new_z.y, new_z.z}});

    return basis_mat.inverse() * point;
}

std::vector<Vec3> transform_z_axis(const std::vector<Vec3> &points,
                                   const Vec3 &new_z)
{
    std::vector<Vec3> result;
    result.resize(points.size());

    std::transform(
        std::cbegin(points), std::cend(points), std::begin(result),
        [new_z](const Vec3 &p) { return transform_z_axis(p, new_z); });

    return result;
}

std::vector<Vec3> points_sorted_ctr_clockwise(const std::vector<Vec3> &points,
                                              Vec3 normal)
{
    std::vector<Vec3> transf_pts = transform_z_axis(points, normal);
    assert(transf_pts.size() == points.size());
    // the center is the average of every transformed point
    Vec3 center =
        std::reduce(transf_pts.begin(), transf_pts.end()) / transf_pts.size();

    std::vector<size_t> idxs(points.size());
    std::iota(idxs.begin(), idxs.end(), 0);

    std::sort(idxs.begin(), idxs.end(),
              [transf_pts, center](size_t lhs_i, size_t rhs_i) {
                  // atan2(y, x) gives the counter-clockwise angle of a point x,
                  // y from the +x axis.
                  assert(lhs_i < transf_pts.size());
                  assert(rhs_i < transf_pts.size());
                  float lhs_angle = std::atan2(transf_pts[lhs_i].y - center.y,
                                               transf_pts[lhs_i].x - center.x);
                  float rhs_angle = std::atan2(transf_pts[rhs_i].y - center.y,
                                               transf_pts[rhs_i].x - center.x);
                  return lhs_angle > rhs_angle;
              });

    std::vector<Vec3> sorted_pts;
    sorted_pts.reserve(points.size());
    for (size_t idx : idxs) {
        sorted_pts.push_back(points[idx]);
    }

    return sorted_pts;
}
*/

void points_sorted_ctr_clockwise(std::span<Vec3> points, Vec3 normal)
{
    if (points.size() == 0)
        return;

    // the center is the average of every transformed point
    Vec3 center = std::reduce(points.begin(), points.end()) / points.size();

    // the angle of every point will be an angle from this vector
    Vec3 angle_basis = center + points[0].project(normal).normalize();

    std::sort(points.begin(), points.end(),
              [normal, angle_basis](const auto &lhs, const auto &rhs) {
                  Angle lhs_angle =
                      signed_angle_between(lhs, angle_basis, normal);
                  Angle rhs_angle =
                      signed_angle_between(rhs, angle_basis, normal);
                  return lhs_angle > rhs_angle;
              });
}

} // namespace

ConvexHull::ConvexHull()
{
    this->polys.reserve(6);

    this->polys.emplace_back(std::array{
        Vec3(-10000.f, -10000.f, -10000.f), Vec3(-10000.f, 10000.f, -10000.f),
        Vec3(-10000.f, 10000.f, 10000.f), Vec3(-10000.f, -10000.f, 10000.f)});
    this->polys.emplace_back(std::array{
        Vec3(10000.f, 10000.f, -10000.f), Vec3(10000.f, -10000.f, -10000.f),
        Vec3(10000.f, -10000.f, 10000.f), Vec3(10000.f, 10000.f, 10000.f)});

    this->polys.emplace_back(std::array{
        Vec3(-10000.f, -10000.f, -10000.f), Vec3(-10000.f, -10000.f, 10000.f),
        Vec3(10000.f, -10000.f, 10000.f), Vec3(10000.f, -10000.f, -10000.f)});
    this->polys.emplace_back(std::array{
        Vec3(10000.f, 10000.f, -10000.f), Vec3(10000.f, 10000.f, 10000.f),
        Vec3(-10000.f, 10000.f, 10000.f), Vec3(-10000.f, 10000.f, -10000.f)});

    this->polys.emplace_back(std::array{
        Vec3(-10000.f, -10000.f, -10000.f), Vec3(10000.f, -10000.f, -10000.f),
        Vec3(10000.f, 10000.f, -10000.f), Vec3(-10000.f, 10000.f, -10000.f)});
    this->polys.emplace_back(std::array{
        Vec3(10000.f, -10000.f, 10000.f), Vec3(-10000.f, -10000.f, 10000.f),
        Vec3(-10000.f, 10000.f, 10000.f), Vec3(10000.f, 10000.f, 10000.f)});
}

void ConvexHull::remove_empty_polys()
{
    std::erase_if(this->polys, [](Polygon &poly) { return poly.vs.empty(); });
}

void ConvexHull::clip(const Plane &plane)
{
    auto intersect_pts = this->plane_intersections(plane);
    points_sorted_ctr_clockwise(intersect_pts, -plane.normal);

    for (auto &poly : this->polys) {
        poly.clip(plane);
    }

    // make a new polygon from the points where the plane intersected the hull,
    // and have it point away from the hull, like all the other polys.
    // this is made after the clipping portion cuz this poly doesn't need to be
    // clipped.
    this->polys.emplace_back(intersect_pts);

    this->remove_empty_polys();
}

std::vector<Vec3> ConvexHull::plane_intersections(const Plane &plane) const
{
    std::vector<Vec3> points;

    for (const auto &poly : this->polys) {
        auto poly_points = poly.plane_intersections(plane);
        for (const auto &point : poly_points)
            points.push_back(point);
    }

    return points;
}

Vec3 ConvexHull::get_center() const
{
    Vec3 sum(0.f, 0.f, 0.f);
    size_t n = 0;

    for (const auto &poly : this->polys) {
        for (const auto &v : poly.vs) {
            sum += v;
            ++n;
        }
    }

    return sum / n;
}

bool ConvexHull::verify_winding_order() const
{
    Vec3 center = this->get_center();

    for (const auto &poly : this->polys) {
        if (!poly.get_plane().is_point_behind(center))
            return false;
    }

    return true;
}

// the point is inside if it's behind every polygon making up the hull
bool ConvexHull::is_point_inside(const Vec3 &p) const
{
    for (const auto &poly : this->polys) {
        Plane plane = poly.get_plane();
        if (!plane.is_point_behind(p))
            return false;
    }

    return true;
}
