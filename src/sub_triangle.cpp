#include "sub_triangle.hpp"
#include "camera.hpp"
#include "frame.hpp"
#include "line.hpp"
#include "render_px.hpp"
#include "resolution.hpp"
#include "rspan.hpp"
#include "texture.hpp"
#include "triangle.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
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
    w.x = std::round((v.x + 1.f) / 2.f * Res::width);
    w.y = std::round((-v.y + 1.f) / 2.f * Res::height);

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

struct TriangleLines {

    // a list of lines from one edge of the triangle to another.
    // each element in starts and ends represents one scanline of the triangle,
    // starting from the top and ending at the bottom.
    std::array<Line1Di, Res::max_height> lines;
    size_t n_lines;
    // how much to add to the index of a scanline to get its y coordinate in
    // screen-space.
    int32_t y_offset;
};

// bresenham line algorithm from https://gist.github.com/bert/1085538
void set_tri_edge_list_via_line(Vec2i start, const Vec2i &end,
                                struct TriangleLines &lines)
{
    int32_t dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1;
    int32_t dy = -abs(end.y - start.y), sy = start.y < end.y ? 1 : -1;
    int32_t err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */

        size_t idx = start.y - lines.y_offset;

        if (idx < lines.n_lines) {
            if (start.x < lines.lines[idx].min_x)
                lines.lines[idx].min_x = start.x;
            if (start.x > lines.lines[idx].max_x)
                lines.lines[idx].max_x = start.x;
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

TriangleLines get_triangle_edge_list(const std::array<Vec2i, 3> &vs)
{
    TriangleLines lines;

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
        lines.lines[i].min_x = std::numeric_limits<int32_t>::max();
        lines.lines[i].max_x = std::numeric_limits<int32_t>::lowest();
    }

    set_tri_edge_list_via_line(vs[0], vs[1], lines);
    set_tri_edge_list_via_line(vs[1], vs[2], lines);
    set_tri_edge_list_via_line(vs[2], vs[0], lines);

    return lines;
}

float tri_2d_area(std::span<const Vec2> vs)
{
    // heron's formula

    float a = vs[0].dist(vs[1]);
    float b = vs[1].dist(vs[2]);
    float c = vs[2].dist(vs[0]);

    float s = (a + b + c) / 2;

    return std::sqrt(s * (s - a) * (s - b) * (s - c));
}

} // namespace

SubTriangle::SubTriangle(std::span<const Vec3, 3> vs,
                         std::span<const Vec2, 3> vts, const Triangle *parent)
    : parent(parent)
{
    for (int i = 0; i < 3; ++i) {
        this->vs[i] = vs[i];
        this->vts[i] = vts[i];
    }
}

std::array<Vec2i, 3> SubTriangle::get_screen_vs() const
{
    return this->screen_vs;
}

void SubTriangle::project_to_scr(const Camera &cam)
{
    auto norm_scr_vs = camera_vs_to_norm_scr(this->vs, cam);
    this->screen_vs = norm_scr_vs_to_scr(norm_scr_vs);
}

void SubTriangle::render(Frame &frame, std::span<const Texture> texs) const
{
    TriangleLines lines = get_triangle_edge_list(this->screen_vs);

    for (size_t i = 0; i < lines.n_lines; i++) {
        int32_t y = i + lines.y_offset;

        if (lines.lines[i].max_x < 0 || lines.lines[i].min_x >= Res::width)
            continue;
        int32_t clpd_min = std::max(lines.lines[i].min_x, 0);
        int32_t clpd_max = std::min(lines.lines[i].max_x, Res::width - 1);

        if (RSpan::enabled) {
            frame.scans[y].add_edge(Edge(this, clpd_min, true));
            frame.scans[y].add_edge(Edge(this, clpd_max, false));
        } else {
            RenderPixels::render_horizontal_line(y, clpd_min, clpd_max - 1,
                                                 frame, *this, texs);
        }
    }
}

bool SubTriangle::is_visible(const Frame &frame) const
{
    TriangleLines lines = get_triangle_edge_list(this->screen_vs);

    for (size_t i = 0; i < lines.n_lines; i++) {
        int32_t y = i + lines.y_offset;

        if (lines.lines[i].max_x < 0 || lines.lines[i].min_x >= Res::width)
            continue;
        int32_t clpd_min = std::max(lines.lines[i].min_x, 0);
        int32_t clpd_max = std::min(lines.lines[i].max_x, Res::width - 1);

        assert(!RSpan::enabled);
        if (RenderPixels::horizontal_line_visible(y, clpd_min, clpd_max - 1,
                                                  frame, *this))
            return true;
    }

    return false;
}

float SubTriangle::tex_space_area() const
{
    return tri_2d_area(this->vts);
}
