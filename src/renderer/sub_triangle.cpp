#include "sub_triangle.hpp"
#include "../resolution.hpp"
#include "triangle.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <memory>

namespace {

Vec2 camera_v_to_norm_scr(const Vec3 &v)
{
    Vec2 w;

    w.x = v.x / v.z;
    w.y = v.y / v.z;

    return w;
}

// camera space -> normalized screen space
std::array<Vec2, 3> camera_vs_to_norm_scr(const std::array<Vec3, 3> &vs)
{
    std::array<Vec2, 3> result;

    for (size_t i = 0; i < vs.size(); ++i) {
        result[i] = camera_v_to_norm_scr(vs[i]);
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
        assert(idx < edges.n_edges);

        if (start.x < edges.starts[idx])
            edges.starts[idx] = start.x;
        if (start.x > edges.ends[idx])
            edges.ends[idx] = start.x;

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

void render_horizontal_line(int y, int x_0, int x_1, const Color &color,
                            Color *frame)
{
    if (y < 0 || y >= static_cast<int>(Res::height))
        return;

    if (x_0 >= static_cast<int>(Res::width) || x_1 < 0)
        return;

    x_0 = std::max(x_0, 0);
    x_1 = std::min(x_1, static_cast<int>(Res::width - 1));

    for (int x = x_0; x <= x_1; ++x) {
        size_t idx = y * Res::width + x;
        assert(idx < Res::size);

        frame[idx] = color;
    }
}

} // namespace

SubTriangle::SubTriangle(std::array<Vec3, 3> vs, const Triangle *parent)
    : vs(vs), parent(parent)
{}

std::array<Vec2i, 3> SubTriangle::get_screen_vs() const
{
    return this->screen_vs;
}

void SubTriangle::project_to_scr()
{
    auto norm_scr_vs = camera_vs_to_norm_scr(this->vs);
    this->screen_vs = norm_scr_vs_to_scr(norm_scr_vs);
}

void SubTriangle::render(Color *frame)
{
    TriangleEdgeList edges = get_triangle_edge_list(this->screen_vs);

    for (size_t i = 0; i < edges.n_edges; i++) {
        int y = i + edges.y_offset;
        render_horizontal_line(y, edges.starts[i], edges.ends[i],
                               this->parent->color, frame);
    }
}
