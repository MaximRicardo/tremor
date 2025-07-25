#include "polygon.hpp"
#include "angle.hpp"
#include "index.hpp"
#include "mat4x4.hpp"
#include "palette.hpp"
#include "plane.hpp"
#include "resolution.hpp"
#include "transform.hpp"
#include "vector/vec2.hpp"
#include "vector/vec3.hpp"
#include "vertex.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <span>
#include <tuple>
#include <utility>

namespace {

constexpr bool ignore_depth_buffer = false;

Angle signed_angle_between(const Vec3 &v, const Vec3 &w, const Vec3 &normal)
{
    float dot = v.dot(w);
    float det = normal.dot(v.cross(w));
    return Angle(std::atan2(det, dot));
}

struct PolyScanline {

    int32_t start_x;
    float start_z;
    Vec2 start_vt;

    int32_t end_x;
    float end_z;
    Vec2 end_vt;
};

struct PolyScanlines {

    std::array<PolyScanline, Res::max_height> lines;
    size_t n_lines;
    // the y position of the first scanline
    int32_t y_offset;
};

float interpolate_z(Vec2i p, const Vec3 &start_cam_space,
                    const Vec3 &end_cam_space, Vec2i start_scr, Vec2i end_scr)
{
    if (start_scr.dist(end_scr) == 0)
        return start_cam_space.z;

    float t_start =
        static_cast<float>(start_scr.dist(p)) / start_scr.dist(end_scr);
    float t_end = 1.f - t_start;

    float z = 1.f / (t_start * (1.f / start_cam_space.z) +
                     t_end * (1.f / end_cam_space.z));
    return z;
}

Vec2 interpolate_vt(Vec2i p, float z, const Vec3 &start_cam_space,
                    const Vec3 &end_cam_space, const Vec2 &start_vt,
                    const Vec2 &end_vt, Vec2i start_scr, Vec2i end_scr)
{
    if (start_scr.dist(end_scr) == 0)
        return start_vt;

    float t_start =
        static_cast<float>(start_scr.dist(p)) / start_scr.dist(end_scr);
    float t_end = 1.f - t_start;

    Vec2 vt = (start_vt / start_cam_space.z * t_start +
               end_vt / end_cam_space.z * t_end) *
              z;
    return vt;
}

float hori_interpolate_z(int32_t x, const PolyScanline &line)
{
    if (1 || line.end_x == line.start_x)
        return line.start_z;

    float t_start =
        static_cast<float>(x - line.start_x) / (line.end_x - line.start_x);
    float t_end = 1.f - t_start;

    float z =
        1.f / (t_start * (1.f / line.start_z) + t_end * (1.f / line.end_z));
    return z;
}

Vec2 hori_interpolate_vt(int32_t x, float z, const PolyScanline &line)
{
    if (1 || line.end_x == line.start_x)
        return line.start_vt;

    float t_start =
        static_cast<float>(x - line.start_x) / (line.end_x - line.start_x);
    float t_end = 1.f - t_start;

    Vec2 vt = (line.start_vt / line.start_z * t_start +
               line.end_vt / line.end_z * t_end) *
              z;
    return vt;
}

// bresenham line algorithm from https://gist.github.com/bert/1085538
void set_poly_line_list_via_line(const Vec2i &start, const Vec2i &end,
                                 const Vec3 &start_cam_space,
                                 const Vec3 &end_cam_space,
                                 const Vec2 &start_vt, const Vec2 &end_vt,
                                 struct PolyScanlines &lines)
{
    int32_t dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1;
    int32_t dy = -abs(end.y - start.y), sy = start.y < end.y ? 1 : -1;
    int32_t err = dx + dy, e2; /* error value e_xy */

    Vec2i pos = start;

    for (;;) { /* loop */

        size_t idx = pos.y - lines.y_offset;

        if (idx < lines.n_lines) {
            float z =
                interpolate_z(pos, start_cam_space, end_cam_space, start, end);
            Vec2 vt = interpolate_vt(pos, z, start_cam_space, end_cam_space,
                                     start_vt, end_vt, start, end);
            if (pos.x < lines.lines[idx].start_x) {
                lines.lines[idx].start_x = pos.x;
                lines.lines[idx].start_z = z;
                lines.lines[idx].start_vt = vt;
            }
            if (pos.x > lines.lines[idx].end_x) {
                lines.lines[idx].end_x = pos.x;
                lines.lines[idx].end_z = z;
                lines.lines[idx].end_vt = vt;
            }
        }

        if (pos.x == end.x && pos.y == end.y)
            break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            pos.x += sx;
        } /* e_xy+e_x > 0 */
        if (e2 <= dx) {
            err += dx;
            pos.y += sy;
        } /* e_xy+e_y < 0 */
    }
}

PolyScanlines get_poly_line_list(const RenderPolygon &poly,
                                 std::span<const Vec3> cam_space_vs,
                                 std::span<const Vec2i> vs)
{
    PolyScanlines lines;

    auto y_min = std::min({vs[0].y, vs[1].y, vs[2].y});
    auto y_max = std::max({vs[0].y, vs[1].y, vs[2].y});

    // skip any edges outside the screen
    if (y_max < 0 || y_min >= Res::height) {
        lines.n_lines = 0;
        return lines;
    }
    y_min = std::max(y_min, 0);
    y_max = std::min(y_max, Res::height - 1);

    lines.y_offset = y_min;
    lines.n_lines = y_max - y_min + 1;
    assert(lines.n_lines <= Res::max_height);

    for (size_t i = 0; i < lines.n_lines; ++i) {
        lines.lines[i].start_x = std::numeric_limits<int32_t>::max();
        lines.lines[i].end_x = std::numeric_limits<int32_t>::lowest();
    }

    for (auto i = vs.begin(); i < vs.end(); ++i) {
        auto prev = i == vs.begin() ? std::prev(vs.end()) : std::prev(i);

        std::cout << "settin from (" << *prev << ")->(" << *i << ")\n";

        size_t prev_idx = std::distance(vs.begin(), prev);
        size_t i_idx = std::distance(vs.begin(), i);

        const Vec3 &prev_cam = cam_space_vs[prev_idx];
        const Vec3 &i_cam = cam_space_vs[i_idx];
        const Vec2 &prev_tex = poly.vs[prev_idx].vt;
        const Vec2 &i_tex = poly.vs[i_idx].vt;
        set_poly_line_list_via_line(*prev, *i, prev_cam, i_cam, prev_tex, i_tex,
                                    lines);
    }

    return lines;
}

Vec2i get_tex_coords(int32_t x, float z, const PolyScanline &line,
                     const Texture &tex, size_t mipmap_lvl)
{
    Vec2 vt = hori_interpolate_vt(x, z, line);

    if (std::isnan(vt.x))
        vt.x = 0.f;
    if (std::isnan(vt.y))
        vt.y = 0.f;

    // tiling
    vt.x = std::fmod(std::abs(vt.x), 1.f);
    vt.y = std::fmod(std::abs(vt.y), 1.f);

    // texture buffers assume y points down
    vt.y = 1.f - vt.y;

    Vec2i real_vt(vt.x * tex.get_width(mipmap_lvl),
                  vt.y * tex.get_height(mipmap_lvl));

    real_vt.x = std::max(0, std::min(tex.get_width(mipmap_lvl) - 1, real_vt.x));
    real_vt.y =
        std::max(0, std::min(tex.get_height(mipmap_lvl) - 1, real_vt.y));

    return real_vt;
}

void render_horizontal_line(const PolyScanline &line, int32_t y, Frame &frame,
                            const RenderPolygon &poly,
                            std::span<const Texture> texs)
{
    if (y < 0 || y >= Res::height)
        return;

    if (line.start_x >= Res::width || line.end_x < 0)
        return;

    int32_t x_0 = std::max(line.start_x, 0);
    int32_t x_1 = std::min(line.end_x, Res::width - 1);

    for (int x = x_0; x <= x_1; ++x) {
        size_t idx = Index::conv_2d_to_1d(Vec2i(x, y), Res::width);
        assert(idx < Res::n_pixels());

        /*
        std::cout << "x = " << x << ", line = " << line.start_x << "->"
                  << line.end_x << "\n";
                  */

        float z = hori_interpolate_z(x, line);
        /*
        std::cout << "z = " << z << "\n";
        std::cout << "start_z = " << line.start_z << ", end_z = " << line.end_z
                  << "\n";
                  */
        if (!ignore_depth_buffer && frame.depths[idx] < z)
            continue;
        frame.depths[idx] = z;

        const Texture &tex = texs[poly.tex_idx];

        size_t mipmap_lvl = 0;
        Vec2i vt = get_tex_coords(x, z, line, tex, mipmap_lvl);
        /*
        std::cout << "vt = (" << vt << ")\n";
        std::cout << "tex size = " << tex.get_width(mipmap_lvl) << "x"
                  << tex.get_height(mipmap_lvl) << "\n";
                  */
        size_t texel = Index::conv_2d_to_1d(vt, tex.get_width(mipmap_lvl));
        frame.pixels[idx] = Palette::palette[tex.get_pixels(mipmap_lvl)[texel]];

        /*
        Vec3 bary_coords = get_barycentric_coords(
            Vec2(x, y), tri.get_screen_vs()[0], tri.get_screen_vs()[1],
            tri.get_screen_vs()[2]);

        float z = interpolate_z(tri, bary_coords);
        if (!ignore_depth_buffer && frame.depths[idx] < z)
            continue;
        frame.depths[idx] = z;

        auto &tex = texs[tri.parent->tex_idx];

        size_t mipmap = select_mipmap(tri, tex);
        auto texel_coord = get_tex_coords(tri, bary_coords, z, tex, mipmap);
        size_t texel = Index::conv_2d_to_1d(texel_coord, tex.get_width(mipmap));

        frame.pixels[idx] = Palette::palette[tex.get_pixels(mipmap)[texel]];
        */
    }
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

void RenderPolygon::render(const Matrix4x4 &transform, Frame &frame,
                           const Camera &cam,
                           std::span<const Texture> texs) const
{
    auto cam_space_vs = this->get_cam_space_vs(transform, cam);
    auto screen_vs = this->get_screen_vs(transform, cam);
    for (auto v = screen_vs.begin(); v < screen_vs.end(); ++v) {
        std::cout << "v[" << std::distance(screen_vs.begin(), v) << "] = ("
                  << *v << ")\n";
    }

    auto lines = get_poly_line_list(*this, cam_space_vs, screen_vs);

    for (size_t i = 0; i < lines.n_lines; ++i) {
        int32_t y = i + lines.y_offset;

        // std::cout << "y = " << y << "\n";
        render_horizontal_line(lines.lines[i], y, frame, *this, texs);
    }

    /*
    frame.pixels[Index::conv_2d_to_1d(screen_vs[0], Res::width)] =
        Color(255, 0, 0);
    frame.pixels[Index::conv_2d_to_1d(screen_vs[1], Res::width)] =
        Color(0, 255, 0);
    frame.pixels[Index::conv_2d_to_1d(screen_vs[2], Res::width)] =
        Color(0, 0, 255);
    frame.pixels[Index::conv_2d_to_1d(screen_vs[3], Res::width)] =
        Color(255, 255, 255);
        */
}
