#include "camera.hpp"
#include "color.hpp"
#include "obj_loader/obj_loader.hpp"
#include "renderer/triangle.hpp"
#include "resolution.hpp"
#include "screen/screen.hpp"
#include "time.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>

int main()
{
    Time::init();

    Screen screen(Res::width, Res::height, Res::upscaled_width,
                  Res::upscaled_height, "Quaken't");

    auto frame = std::make_unique<Color[]>(Res::size);

    Camera cam(Vec3(0.f, 0.f, 0.f));

    auto tris = ObjLoader::load_file("../objs/cube.obj");
    for (size_t i = 0; i < tris.size(); i++) {
        tris[i].color = Color(i % 2 == 0 ? 128 : 255, i % 3 == 0 ? 0 : 255,
                              i % 4 == 0 ? 0 : 255);
    }

    uint32_t prev_time = Time::get_ticks_ms();
    while (!screen.should_close()) {
        float delta_time =
            static_cast<float>(Time::get_ticks_ms() - prev_time) / 1000.f;
        // fps is capped to 1000 cuz get_ticks_ms doesn't have any higher
        // accuracy
        if (delta_time < 0.001f)
            continue;

        prev_time = Time::get_ticks_ms();

        printf("delta_time = %f\n", delta_time);

        for (std::size_t i = 0; i < Res::size; i++) {
            frame[i] = Color(0, 0, 0);
        }

        cam.handle_input(delta_time, screen);

        for (auto &tri : tris) {
            tri.render(frame.get(), cam);
        }

        screen.update(frame.get());
    }
}
