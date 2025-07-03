#include "color.hpp"
#include "resolution.hpp"
#include "screen/screen.hpp"
#include <cstddef>
#include <cstdio>
#include <memory>

int main()
{
    Screen screen(Res::width, Res::height, Res::upscaled_width,
                  Res::upscaled_height, "Quaken't");

    auto frame = std::make_unique<Color[]>(Res::size);

    while (!screen.should_close()) {

        for (std::size_t i = 0; i < Res::size; i++) {
            frame[i].r += 1;
        }

        screen.update(frame.get());
    }
}
