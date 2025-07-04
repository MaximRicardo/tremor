#include "color.hpp"
#include "resolution.hpp"
#include "screen/screen.hpp"
#include "triangle/triangle.hpp"
#include <cstddef>
#include <cstdio>
#include <memory>

int main()
{
    Screen screen(Res::width, Res::height, Res::upscaled_width,
                  Res::upscaled_height, "Quaken't");

    auto frame = std::make_unique<Color[]>(Res::size);

    Triangle tri(Vec3(-1.f, -1.f, 1.f), Vec3(1.f, -1.f, 1.f),
                 Vec3(0.f, 1.f, 1.f));

    while (!screen.should_close()) {

        for (std::size_t i = 0; i < Res::size; i++) {
            frame[i] = Color(0, 0, 0);
        }

        tri.render(frame.get());

        screen.update(frame.get());
    }
}
