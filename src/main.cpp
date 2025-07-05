#include "camera.hpp"
#include "color.hpp"
#include "obj_loader/obj_loader.hpp"
#include "renderer/triangle.hpp"
#include "resolution.hpp"
#include "screen/screen.hpp"
#include "texture.hpp"
#include "time.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

int main()
{
    Time::init();

    Screen screen(Res::width, Res::height, Res::upscaled_width,
                  Res::upscaled_height, "Quaken't");

    auto frame = std::make_unique<Color[]>(Res::size);
    auto depth_buffer = std::make_unique<float[]>(Res::size);

    Camera cam(Vec3(0.f, 0.f, -2.f));

    auto tris = ObjLoader::load_file("../objs/cube.obj");

    std::vector<Texture> texs(1);
    texs[0].load("../textures/img.png");

    printf("n pixels = %zu, dims = %ux%u\n", texs[0].get_pixels().size(),
           texs[0].get_width(), texs[0].get_height());

    uint32_t prev_time = Time::get_ticks_ms();
    while (!screen.should_close()) {
        float delta_time =
            static_cast<float>(Time::get_ticks_ms() - prev_time) / 1000.f;
        // fps is capped to 1000 cuz get_ticks_ms doesn't have enough accuracy
        // for anything higher than that
        if (delta_time < 0.001f)
            continue;

        prev_time = Time::get_ticks_ms();

        printf("delta_time = %f\n", delta_time);

        for (std::size_t i = 0; i < Res::size; i++) {
            frame[i] = Color(0, 0, 0);
            depth_buffer[i] = 10000.f;
        }

        cam.handle_input(delta_time, screen);

        for (auto &tri : tris) {
            tri.render(frame.get(), depth_buffer.get(), cam, texs.data());
        }

        screen.update(frame.get());
    }
}
