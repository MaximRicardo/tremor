#include "render_px.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "index.hpp"
#include "palette.hpp"
#include "resolution.hpp"
#include "ssize.hpp"
#include "sub_triangle.hpp"
#include "transform.hpp"
#include "triangle.hpp"
#include "vector/vec2.hpp"
#include "vector/vec3.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>

namespace {

// this will make entities render over each other in wonky ways.
constexpr bool ignore_depth_buffer = false;
constexpr float depth_epsilon = 1.f;

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

    // textures wrap around
    p_tex_coord.x = std::fmod(std::abs(p_tex_coord.x), 1.f);
    p_tex_coord.y = std::fmod(std::abs(p_tex_coord.y), 1.f);

    // flip around the y axis cuz the texture buffers will be assuming Y points
    // down instead of up.
    p_tex_coord.y = 1.f - p_tex_coord.y;

    Vec2i tx = Vec2i(p_tex_coord.x * tex.get_width(mipmap_lvl),
                     p_tex_coord.y * tex.get_height(mipmap_lvl));

    tx.x = std::max(0, std::min(tex.get_width(mipmap_lvl) - 1, tx.x));
    tx.y = std::max(0, std::min(tex.get_height(mipmap_lvl) - 1, tx.y));

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

float tri_2d_area(std::span<const Vec2> vs)
{
    // heron's formula

    float a = vs[0].dist(vs[1]);
    float b = vs[1].dist(vs[2]);
    float c = vs[2].dist(vs[0]);

    float s = (a + b + c) / 2;

    return std::sqrt(s * (s - a) * (s - b) * (s - c));
}
*/

size_t select_mipmap(const SubTriangle &tri, const Texture &tex)
{
    // temporary solution

    float ratio =
        Res::n_pixels() / (tex.n_pixels(0) * tri.tex_space_area() * 2.f);
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

    // nah bro this mipmapping is so ass i'm turning it off
    lvl = 0;

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

} // namespace

// tri               - the triangle the line belongs to
void RenderPixels::render_horizontal_line(int32_t y, int32_t x_0, int32_t x_1,
                                          Frame &frame, const SubTriangle &tri,
                                          std::span<const Texture> texs)
{
    if (y < 0 || y >= Res::height)
        return;

    if (x_0 >= Res::width || x_1 < 0)
        return;

    x_0 = std::max(x_0, 0);
    x_1 = std::min(x_1, Res::width - 1);

    for (int32_t x = x_0; x <= x_1; ++x) {
        size_t idx = Index::to_1d(Vec2i(x, y), Res::width);
        assert(idx < Res::n_pixels());

        Vec3 bary_coords = get_barycentric_coords(
            Vec2(x, y), tri.get_screen_vs()[0], tri.get_screen_vs()[1],
            tri.get_screen_vs()[2]);

        float z = interpolate_z(tri, bary_coords);
        if (!ignore_depth_buffer && frame.depths[idx] < z + depth_epsilon)
            continue;
        frame.depths[idx] = z;

        auto &tex = texs[tri.parent->tex_idx];

        size_t mipmap = select_mipmap(tri, tex);
        auto texel_coord = get_tex_coords(tri, bary_coords, z, tex, mipmap);
        size_t texel = Index::conv_2d_to_1d(texel_coord, tex.get_width(mipmap));

        frame.pixels[idx] = Palette::palette[tex.get_pixels(mipmap)[texel]];
    }
}

bool RenderPixels::horizontal_line_visible(int32_t y, int32_t x_0, int32_t x_1,
                                           const Frame &frame,
                                           const SubTriangle &tri)
{
    if (y < 0 || y >= Res::height)
        return false;

    if (x_0 >= Res::width || x_1 < 0)
        return false;

    x_0 = std::max(x_0, 0);
    x_1 = std::min(x_1, Res::width - 1);

    for (int32_t x = x_0; x <= x_1; ++x) {
        size_t idx = Index::to_1d(Vec2i(x, y), Res::width);
        assert(idx < Res::n_pixels());

        Vec3 bary_coords = get_barycentric_coords(
            Vec2(x, y), tri.get_screen_vs()[0], tri.get_screen_vs()[1],
            tri.get_screen_vs()[2]);

        float z = interpolate_z(tri, bary_coords);
        if (ignore_depth_buffer || z + depth_epsilon < frame.depths[idx])
            return true;
    }

    return false;
}

void RenderPixels::render_point(const Vec3 &p, Frame &frame, const Camera &cam,
                                Color color, int32_t radius, bool ignore_depths)
{
    Vec3 cs = Transform::world_space_to_cam_space(p, cam);
    if (cs.z < Consts::z_near)
        return;

    Vec2 nss = Transform::cam_space_to_norm_scr(cs, cam);
    Vec2i ss = Transform::norm_scr_to_scr_space(nss);

    // just draw a square for now
    for (int32_t y = ss.y - radius; y < ss.y + radius; ++y) {
        for (int32_t x = ss.x - radius; x < ss.x + radius; ++x) {
            if (x < 0 || x >= Res::width)
                return;
            if (y < 0 || y >= Res::height)
                return;

            isize_t idx = Index::to_1d(Vec2i(x, y), Res::width);
            if (!ignore_depth_buffer && !ignore_depths &&
                frame.depths[idx] < cs.z + depth_epsilon)
                return;
            frame.depths[idx] = cs.z;
            frame.pixels[idx] = color;
        }
    }
}
