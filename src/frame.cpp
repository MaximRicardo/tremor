#include "frame.hpp"
#include "constants.hpp"
#include "resolution.hpp"
#include <algorithm>

Frame::Frame() : pixels(Res::n_pixels()), depths(Res::n_pixels()) {}

void Frame::clear()
{
    std::fill(this->pixels.begin(), this->pixels.end(), Color(0, 0, 0));
    std::fill(this->depths.begin(), this->depths.end(), Consts::z_far);
}

void Frame::update_resolution()
{
    pixels = FixedArray<Color>(Res::n_pixels());
    depths = FixedArray<float>(Res::n_pixels());
}
