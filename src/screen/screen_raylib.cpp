#ifdef m_RAYLIB

#include "../resolution.hpp"
#include "screen.hpp"
#include <raylib.h>
#include <string>

class ScreenInfo {

public:
    Texture screen_tex;
};

namespace {

Texture init_screen_tex()
{
    Image img = GenImageColor(Res::width, Res::height, BLACK);
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    Texture screen_tex = LoadTextureFromImage(img);

    UnloadImage(img);

    return screen_tex;
}

void resize_screen_tex(Texture &screen_tex)
{
    UnloadTexture(screen_tex);
    screen_tex = init_screen_tex();
}

} // namespace

Screen::Screen(std::string name)
{
    this->info = new ScreenInfo;

    SetTraceLogLevel(LOG_ERROR);

    InitWindow(Res::upscaled_width, Res::upscaled_height, name.c_str());

    this->info->screen_tex = init_screen_tex();
}

Screen::~Screen()
{
    UnloadTexture(this->info->screen_tex);
    delete this->info;
    CloseWindow();
}

void Screen::update(const void *pixels)
{
    Rectangle src_rect = {0, (float)Res::height, (float)Res::width,
                          (float)Res::height};
    Rectangle dest_rect = {0, 0, (float)Res::upscaled_width,
                           (float)Res::upscaled_height};

    UpdateTexture(this->info->screen_tex, pixels);

    BeginDrawing();
    DrawTexturePro(this->info->screen_tex, src_rect, dest_rect, {0.f, 0.f}, 0.f,
                   WHITE);
    EndDrawing();
}

void Screen::update_resolution()
{
    resize_screen_tex(this->info->screen_tex);
    SetWindowSize(Res::upscaled_width, Res::upscaled_height);
}

bool Screen::should_close() const
{
    return WindowShouldClose();
}

#endif
