#include "bsp.hpp"
#include "camera.hpp"
#include "frame.hpp"
#include "input/input.hpp"
#include "map_loading/quake_map.hpp"
#include "resolution.hpp"
#include "screen/screen.hpp"
#include "time.hpp"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {

constexpr bool render_bsp_tree = true;

void resize_window(uint32_t width, uint32_t height, uint32_t upscaled_width,
                   uint32_t upscaled_height, Screen &screen, Frame &frame)
{
    Res::width = width;
    Res::height = height;
    Res::upscaled_width = upscaled_width;
    Res::upscaled_height = upscaled_height;

    frame.update_resolution();
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

    Map map;
    std::filesystem::path map_path = "../maps/e1m1.map";

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

    std::cout << "map has " << map.tris.size()
              << " triangles before BSP generation\n";
    std::cout << "map has " << map.n_triangles()
              << " triangles after BSP generation\n";
    std::cout << "worldspawn max depth is " << worldspawn.bsp->max_depth()
              << "\n";

    Camera cam(map.get_player_start(), Angle(0.f), Angle(0.f),
               Angle(90.f, Angle::Type::DEGREES), 100.f);

    Frame frame;
    uint32_t prev_time = Time::get_ticks_ms();
    while (!screen.should_close()) {
        float delta_time =
            static_cast<float>(Time::get_ticks_ms() - prev_time) / 1000.f;
        // fps is capped to 1000 cuz get_ticks_ms doesn't have enough accuracy
        // for anything higher than that
        if (delta_time < 0.001f)
            continue;

        prev_time = Time::get_ticks_ms();

        frame.clear();

        /*
        std::cout << "delta_time = " << delta_time << '\n';
        std::cout << "fps = " << 1.f / delta_time << '\n';
        bool cam_in_solid = worldspawn.bsp->point_in_solid(cam.pos, worldspawn);
        std::cout << "cam in solid = " << cam_in_solid << '\n';
        */

        cam.handle_input(delta_time, screen);
        if (Input::key_pressed_once(Input::Key::R, screen))
            resize_window(320, 200, Res::upscaled_width, Res::upscaled_height,
                          screen, frame);
        else if (Input::key_pressed_once(Input::Key::F, screen))
            resize_window(160, 100, Res::upscaled_width, Res::upscaled_height,
                          screen, frame);

        if (render_bsp_tree) {
            map.render(frame, cam);
        } else {
            for (auto &tri : map.tris) {
                if (tri.get_plane().is_point_behind(cam.pos))
                    continue;
                tri.render(Matrix4x4::identity(), frame, cam, map.textures);
            }
        }

        screen.update(frame.pixels.data());
    }

    return 0;
}
