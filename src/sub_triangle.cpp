#include "sub_triangle.hpp"
#include "camera.hpp"
#include "index.hpp"
#include "palette.hpp"
#include "resolution.hpp"
#include "texture.hpp"
#include "triangle.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <memory>
#include <span>

namespace {

Vec2 camera_v_to_norm_scr(const Vec3 &v, const Camera &cam)
{
    float x_fov_mult = 1.f / std::tan(cam.hfov.get() / 2.f);
    float y_fov_mult = 1.f / std::tan(cam.vfov.get() / 2.f);

    Vec2 w;

    w.x = v.x * x_fov_mult / v.z;
    w.y = v.y * y_fov_mult / v.z;

    return w;
}

// camera space -> normalized screen space
std::array<Vec2, 3> camera_vs_to_norm_scr(const std::array<Vec3, 3> &vs,
                                          const Camera &cam)
{
    std::array<Vec2, 3> result;

    for (size_t i = 0; i < vs.size(); ++i) {
        result[i] = camera_v_to_norm_scr(vs[i], cam);
    }

    return result;
}

Vec2i norm_scr_v_to_scr(const Vec2 &v)
{
    Vec2i w;
    w.x = (v.x + 1.f) / 2.f * Res::width;
    w.y = (-v.y + 1.f) / 2.f * Res::height;

    return w;
}

// normalized screen space -> screen space
std::array<Vec2i, 3> norm_scr_vs_to_scr(const std::array<Vec2, 3> &vs)
{
    std::array<Vec2i, 3> result;

    for (std::size_t i = 0; i < vs.size(); i++) {
        result[i] = norm_scr_v_to_scr(vs[i]);
    }

    return result;
}

struct TriangleEdgeList {

    // a list of lines from one edge of the triangle to another.
    // each element in starts and ends represents one scanline of the triangle,
    // starting from the top and ending at the bottom.
    std::unique_ptr<int[]> starts;
    std::unique_ptr<int[]> ends;
    size_t n_edges;
    // how much to add to the index of a scanline to get its y coordinate in
    // screen-space.
    int y_offset;
};

void set_tri_edge_list_via_line(Vec2i start, const Vec2i &end,
                                struct TriangleEdgeList &edges)
{
    // bresenham line algorithm from https://gist.github.com/bert/1085538

    int dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1;
    int dy = -abs(end.y - start.y), sy = start.y < end.y ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */

        size_t idx = start.y - edges.y_offset;

        if (idx < edges.n_edges) {
            if (start.x < edges.starts[idx])
                edges.starts[idx] = start.x;
            if (start.x > edges.ends[idx])
                edges.ends[idx] = start.x;
        }

        if (start.x == end.x && start.y == end.y)
            break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            start.x += sx;
        } /* e_xy+e_x > 0 */
        if (e2 <= dx) {
            err += dx;
            start.y += sy;
        } /* e_xy+e_y < 0 */
    }
}

TriangleEdgeList get_triangle_edge_list(const std::array<Vec2i, 3> &vs)
{
    TriangleEdgeList edges;

    int y_min = std::min({vs[0].y, vs[1].y, vs[2].y});
    int y_max = std::max({vs[0].y, vs[1].y, vs[2].y});

    // skip any edges outside the screen
    if (y_max < 0 || y_min >= static_cast<int>(Res::height)) {
        edges.n_edges = 0;
        return edges;
    }
    y_min = std::max(y_min, 0);
    y_max = std::min(y_max, static_cast<int>(Res::height - 1));

    edges.y_offset = y_min;
    edges.n_edges = y_max - y_min + 1;
    edges.starts = std::make_unique<int[]>(edges.n_edges);
    edges.ends = std::make_unique<int[]>(edges.n_edges);

    for (size_t i = 0; i < edges.n_edges; ++i) {
        edges.starts[i] = std::numeric_limits<int>::max();
        edges.ends[i] = std::numeric_limits<int>::lowest();
    }

    set_tri_edge_list_via_line(vs[0], vs[1], edges);
    set_tri_edge_list_via_line(vs[1], vs[2], edges);
    set_tri_edge_list_via_line(vs[2], vs[0], edges);

    return edges;
}

// (u, v, w) are mapped to x, y, z
// https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
Vec3 get_barycentric_coords(Vec2 p, Vec2 a, Vec2 b, Vec2 c)
{
    Vec2 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);
    float denom = d00 * d11 - d01 * d01;

    Vec3 ret;
    ret.y = (d11 * d20 - d01 * d21) / (denom + 0.001f);
    ret.z = (d00 * d21 - d01 * d20) / (denom + 0.001f);
    ret.x = 1.f - ret.y - ret.z;

    return ret;
}

float interpolate_z(const SubTriangle &tri, const Vec3 &bary_coords)
{
    float z = 1.f / (1.f / tri.vs[0].z * bary_coords.x +
                     1.f / tri.vs[1].z * bary_coords.y +
                     1.f / tri.vs[2].z * bary_coords.z);
    return z;
}

Vec2i get_tex_coords(const SubTriangle &tri, const Vec3 &bary_coords, float z,
                     const Texture &tex, size_t mipmap_lvl)
{
    assert(tri.vs[0].z != 0.f);
    assert(tri.vs[1].z != 0.f);
    assert(tri.vs[2].z != 0.f);

    Vec2 p_tex_coord = (tri.vts[0] / tri.vs[0].z * bary_coords.x +
                        tri.vts[1] / tri.vs[1].z * bary_coords.y +
                        tri.vts[2] / tri.vs[2].z * bary_coords.z) *
                       z;

    if (std::isnan(p_tex_coord.x))
        p_tex_coord.x = 0.f;
    if (std::isnan(p_tex_coord.y))
        p_tex_coord.y = 0.f;

    // flip around the y axis cuz the texture buffers will be assuming Y points
    // down instead of up.
    p_tex_coord.y = 1.f - p_tex_coord.y;

    Vec2i tx = Vec2i(p_tex_coord.x * tex.get_width(mipmap_lvl),
                     p_tex_coord.y * tex.get_height(mipmap_lvl));

    tx.x = std::max(
        0, std::min(static_cast<int>(tex.get_width(mipmap_lvl) - 1), tx.x));
    tx.y = std::max(
        0, std::min(static_cast<int>(tex.get_height(mipmap_lvl) - 1), tx.y));

    return tx;
}

/*
int32_t tri_2d_area(std::span<const Vec2i> vs)
{
    // heron's formula

    int32_t a = vs[0].dist(vs[1]);
    int32_t b = vs[1].dist(vs[2]);
    int32_t c = vs[2].dist(vs[0]);

    int32_t s = (a + b + c) / 2;

    return std::sqrt(s * (s - a) * (s - b) * (s - c));
}
*/

float tri_2d_area(std::span<const Vec2> vs)
{
    // heron's formula

    float a = vs[0].dist(vs[1]);
    float b = vs[1].dist(vs[2]);
    float c = vs[2].dist(vs[0]);

    float s = (a + b + c) / 2;

    return std::sqrt(s * (s - a) * (s - b) * (s - c));
}

size_t select_mipmap(const SubTriangle &tri, const Texture &tex)
{
    // temporary solution

    float ratio = Res::size / (tex.n_pixels(0) * tri.tex_space_area() * 2.f);
    float avrg_z = (tri.vs[0].z + tri.vs[1].z + tri.vs[2].z) / 3.f;

    size_t lvl;
    // these values were eyeballed and could probably be improved
    float mult = 7.f;
    if (avrg_z < 1.5f * ratio * mult)
        lvl = 0;
    else if (avrg_z < 3.5f * ratio * mult)
        lvl = 1;
    else if (avrg_z < 12.f * ratio * mult)
        lvl = 2;
    else
        lvl = 3;

    return std::min(lvl, tex.n_mipmap_lvls);

    // can't manage to get this to work

    /*
    int32_t area = tri_2d_area(tri.get_screen_vs());

    float ratio = static_cast<float>(area) / tri.tex_space_area() /
                  (static_cast<float>(Res::size));

    if (ratio > 0.5f)
        return 0;
    else if (ratio > 0.1f)
        return 1;
    else if (ratio > 0.01f)
        return 2;
    else
        return 3;
    */
}

//
// tri               - the triangle the line belongs to
void render_horizontal_line(int y, int x_0, int x_1, std::span<Color> frame,
                            std::span<float> depth_buffer,
                            const SubTriangle &tri,
                            std::span<const Texture> texs)
{
    if (y < 0 || y >= static_cast<int>(Res::height))
        return;

    if (x_0 >= static_cast<int>(Res::width) || x_1 < 0)
        return;

    x_0 = std::max(x_0, 0);
    x_1 = std::min(x_1, static_cast<int>(Res::width - 1));

    for (int x = x_0; x <= x_1; ++x) {
        size_t idx = Index::conv_2d_to_1d(Vec2i(x, y), Res::width);
        assert(idx < Res::size);

        Vec3 bary_coords = get_barycentric_coords(
            Vec2(x, y), tri.get_screen_vs()[0], tri.get_screen_vs()[1],
            tri.get_screen_vs()[2]);

        float z = interpolate_z(tri, bary_coords);
#ifndef m_DO_NOT_CHECK_DEPTH_BUFFER
        if (depth_buffer[idx] < z)
            continue;
#endif
        depth_buffer[idx] = z;

        auto &tex = texs[tri.parent->tex_idx];

        size_t mipmap = select_mipmap(tri, tex);
        auto texel_coord = get_tex_coords(tri, bary_coords, z, tex, mipmap);
        size_t texel = Index::conv_2d_to_1d(texel_coord, tex.get_width(mipmap));

        frame[idx] = Palette::palette[tex.get_pixels(mipmap)[texel]];
    }
}

} // namespace

SubTriangle::SubTriangle(std::array<Vec3, 3> vs, std::array<Vec2, 3> vts,
                         const Triangle *parent)
    : vs(vs), vts(vts), parent(parent)
{}

std::array<Vec2i, 3> SubTriangle::get_screen_vs() const
{
    return this->screen_vs;
}

void SubTriangle::project_to_scr(const Camera &cam)
{
    auto norm_scr_vs = camera_vs_to_norm_scr(this->vs, cam);
    this->screen_vs = norm_scr_vs_to_scr(norm_scr_vs);
}

void SubTriangle::render(std::span<Color> frame, std::span<float> depth_buffer,
                         std::span<const Texture> texs)
{
    TriangleEdgeList edges = get_triangle_edge_list(this->screen_vs);

    for (size_t i = 0; i < edges.n_edges; i++) {
        int y = i + edges.y_offset;
        render_horizontal_line(y, edges.starts[i], edges.ends[i], frame,
                               depth_buffer, *this, texs);
    }
}

float SubTriangle::tex_space_area() const
{
    return tri_2d_area(this->vts);
}
