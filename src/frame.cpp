#include "frame.hpp"
#include "constants.hpp"
#include "edge.hpp"
#include "resolution.hpp"
#include <algorithm>
#include <cassert>
#include <iterator>

void Scanline::clear()
{
    this->edges.clear();
}

void Scanline::add_edge(const Edge &edge)
{
    if (this->edges.empty()) {
        this->edges.push_front(edge);
        return;
    }

    for (auto cur = this->edges.begin(); cur != this->edges.end(); ++cur) {
        bool is_last = cur == std::prev(this->edges.end());

        if (is_last || cur->x < edge.x) {
            this->edges.insert(std::next(cur), edge);
            break;
        }
    }
}

Frame::Frame()
    : pixels(Res::n_pixels()), depths(Res::n_pixels()), scans(Res::height)
{}

void Frame::clear()
{
    std::fill(this->pixels.begin(), this->pixels.end(), Color(0, 0, 0));
    std::fill(this->depths.begin(), this->depths.end(), Consts::z_far);

    for (auto &scan : this->scans)
        scan.clear();
}

void Frame::update_resolution()
{
    pixels = FixedArray<Color>(Res::n_pixels());
    depths = FixedArray<float>(Res::n_pixels());

    scans = FixedArray<Scanline>(Res::height);
}
