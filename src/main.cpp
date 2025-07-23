#include "bsp.hpp"
#include "camera.hpp"
#include "color.hpp"
#include "map_loading/quake_map.hpp"
#include "resolution.hpp"
#include "screen/screen.hpp"
#include "time.hpp"
#include "utils/fixed_array.hpp"
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <stdexcept>

int main()
{
    Time::init();

    Screen screen(Res::width, Res::height, Res::upscaled_width,
                  Res::upscaled_height, "Quaken't");

    FixedArray<Color> frame(Res::size);
    FixedArray<float> depth_buffer(Res::size);

    Camera cam(Vec3(0.f, 0.f, -2.f), Angle(0.f), Angle(0.f),
               Angle(90.f, Angle::Type::DEGREES));

    Map map;
    std::filesystem::path map_path = "../maps/map.map";
    QuakeMapLoader::Format fmt = QuakeMapLoader::Format::QUAKE_1;

    try {
        map = QuakeMapLoader::load_file(map_path, fmt);
    } catch (std::runtime_error &e) {
        std::cerr << "failed to load in .map file '" << map_path.string()
                  << "': " << e.what() << "\n";
        return 1;
    }

    std::cout << "map loaded\n";

    MapEntity &worldspawn = map.entities[0];

    std::cout << "map has " << map.n_triangles() << " tris\n";
    std::cout << "worldspawn max depth is " << worldspawn.bsp.max_depth()
              << "\n";

    uint32_t prev_time = Time::get_ticks_ms();
    while (!screen.should_close()) {
        float delta_time =
            static_cast<float>(Time::get_ticks_ms() - prev_time) / 1000.f;
        // fps is capped to 1000 cuz get_ticks_ms doesn't have enough accuracy
        // for anything higher than that
        if (delta_time < 0.001f)
            continue;

        prev_time = Time::get_ticks_ms();

        std::cout << "delta_time = " << delta_time << '\n';
        bool cam_in_solid = worldspawn.bsp.point_in_solid(cam.pos, worldspawn);
        std::cout << "cam in solid = " << cam_in_solid << '\n';

        for (std::size_t i = 0; i < Res::size; i++) {
            frame[i] = Color(0, 0, 0);
            depth_buffer[i] = 10000.f;
        }

        cam.handle_input(delta_time, screen);

        /*
        for (auto &tri : tris) {
            if (tri.get_plane().is_point_behind(cam.pos))
                continue;
            tri.render(frame, depth_buffer, cam, texs);
        }
        */
        map.render(frame, depth_buffer, cam);

        screen.update(frame.data());
    }

    return 0;
}
