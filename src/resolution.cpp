#include "resolution.hpp"
#include <cstddef>

size_t Res::n_pixels()
{
    return width * height;
}

float Res::aspect_ratio()
{
    return static_cast<float>(width) / height;
}

size_t Res::upscaled_n_pixels()
{
    return upscaled_width * upscaled_height;
}
