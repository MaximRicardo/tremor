#include "bsp.hpp"
#include "camera.hpp"
#include "color.hpp"
#include "input/input.hpp"
#include "map_loading/quake_map.hpp"
#include "resolution.hpp"
#include "screen/screen.hpp"
#include "time.hpp"
#include "utils/fixed_array.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {

void resize_window(uint32_t width, uint32_t height, uint32_t upscaled_width,
                   uint32_t upscaled_height, Screen &screen,
                   FixedArray<Color> &frame, FixedArray<float> &depth_buffer)
{
    Res::width = width;
    Res::height = height;
    Res::upscaled_width = upscaled_width;
    Res::upscaled_height = upscaled_height;

    frame = FixedArray<Color>(Res::n_pixels());
    depth_buffer = FixedArray<float>(Res::n_pixels());

    screen.update_resolution();
}

} // namespace

int main(int argc, char *argv[])
{
    Time::init();

    if (argc >= 2 && std::strcmp(argv[1], "--fast") == 0) {
        // i'd rather not debug the program at a whopping 5 fps
        Res::width /= 2;
        Res::height /= 2;
    }

    Screen screen("Tremor");

    FixedArray<Color> frame(Res::n_pixels());
    FixedArray<float> depth_buffer(Res::n_pixels());

    Map map;
    std::filesystem::path map_path = "../maps/map.map";

    try {
        map = QuakeMapLoader::load_file(map_path);
    } catch (std::runtime_error &e) {
        std::cerr << "failed to load in .map file '" << map_path.string()
                  << "': " << e.what() << "\n";
        return 1;
    }

    std::cout << "map loaded\n";
    std::cout << "n entities = " << map.entities.size() << "\n";

    MapEntity &worldspawn = map.entities[0];

    std::cout << "map has " << map.n_triangles() << " tris\n";
    std::cout << "worldspawn max depth is " << worldspawn.bsp->max_depth()
              << "\n";

    Camera cam(map.get_player_start(), Angle(0.f), Angle(0.f),
               Angle(90.f, Angle::Type::DEGREES), 100.f);

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
        bool cam_in_solid = worldspawn.bsp->point_in_solid(cam.pos, worldspawn);
        std::cout << "cam in solid = " << cam_in_solid << '\n';

        for (std::size_t i = 0; i < Res::n_pixels(); i++) {
            frame[i] = Color(0, 0, 0);
            depth_buffer[i] = 10000.f;
        }

        cam.handle_input(delta_time, screen);
        if (Input::key_pressed_once(Input::Key::R, screen))
            resize_window(320, 200, Res::upscaled_width, Res::upscaled_height,
                          screen, frame, depth_buffer);
        else if (Input::key_pressed_once(Input::Key::F, screen))
            resize_window(160, 100, Res::upscaled_width, Res::upscaled_height,
                          screen, frame, depth_buffer);

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
