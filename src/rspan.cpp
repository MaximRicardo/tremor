#include "rspan.hpp"
#include "edge.hpp"
#include "frame.hpp"
#include "render_px.hpp"
#include "ssize.hpp"
#include "sub_triangle.hpp"
#include "triangle.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <list>
#include <span>
#include <vector>

namespace {

// pre-allocated space to prevent too many malloc invocations
std::vector<const SubTriangle *> apl;

isize_t get_sort_key(const Edge &edge)
{
    return edge.parent->parent->sort_key;
}

bool edge_frontmost_at_px(const std::list<Edge> &list,
                          std::list<Edge>::const_iterator edge)
{
    auto x_start = edge;
    while (x_start != list.begin() && std::prev(x_start)->x == edge->x)
        x_start = std::prev(x_start);

    for (auto other = x_start; other != list.end() && other->x == edge->x;
         ++other) {
        if (other == edge)
            continue;
        else if (other->is_start != edge->is_start)
            continue;

        if (get_sort_key(*other) < get_sort_key(*edge))
            return false;
    }

    return true;
}

void add_tri_to_apl(std::vector<const SubTriangle *> apl,
                    const SubTriangle &tri)
{
    auto insert_p = apl.begin();

    for (auto other = apl.rbegin(); other < apl.rend(); ++other) {
        if ((*other)->parent->sort_key > tri.parent->sort_key) {
            insert_p = other.base();
            break;
        }
    }

    apl.insert(insert_p, &tri);
}

void remove_tri_from_apl(std::vector<const SubTriangle *> apl,
                         const SubTriangle &tri)
{
    for (auto other = apl.begin(); other < apl.end(); ++other) {
        if (*other == &tri) {
            apl.erase(other);
            break;
        }
    }
}

void render_scanline(const Scanline &scan, int32_t y, Frame &frame,
                     std::span<const Texture> texs)
{
    int32_t span_start_x = 0;
    for (auto edge = scan.edges.begin(); edge != scan.edges.end(); ++edge) {
        bool front = edge_frontmost_at_px(scan.edges, edge);

        if (edge->is_start) {
            if (front) {
                if (!apl.empty()) {
                    RenderPixels::render_horizontal_line(
                        y, span_start_x, edge->x, frame, *apl.back(), texs);
                }
                apl.push_back(edge->parent);
                span_start_x = edge->x;
            } else {
                if (apl.empty())
                    span_start_x = edge->x;
                add_tri_to_apl(apl, *edge->parent);
            }
        } else {
            if (front) {
                if (apl.empty())
                    apl.push_back(edge->parent);
                RenderPixels::render_horizontal_line(y, span_start_x, edge->x,
                                                     frame, *apl.back(), texs);
                span_start_x = edge->x + 1;
                remove_tri_from_apl(apl, *edge->parent);
            } else {
                if (!apl.empty())
                    remove_tri_from_apl(apl, *edge->parent);
            }
        }
    }
}

} // namespace

void RSpan::render(Frame &frame, std::span<const Texture> texs)
{
    std::cout << "func enter\n";

    apl.clear();

    for (int32_t y = 0; y < std::ssize(frame.scans); ++y) {
        // std::cout << "y = " << y << "\n";
        render_scanline(frame.scans[y], y, frame, texs);
        apl.clear();
    }

    std::cout << "func exit\n";
}
