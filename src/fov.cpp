#include "fov.hpp"
#include "resolution.hpp"
#include <cmath>

Angle FOV::horizontal_to_vertical(Angle hfov)
{
    // if you want an explanation for how tf this works, i can't give u one cuz
    // i just stole this from one of my old projects and i've long since
    // forgotten how i cooked this up.
    float aspect_ratio =
        static_cast<float>(Res::width) / static_cast<float>(Res::height);
    return Angle(2.f * std::atan(std::tan(hfov.get() / 2.f) / aspect_ratio));
}
