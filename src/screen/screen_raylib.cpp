#ifdef m_RAYLIB

#include "screen.hpp"
#include <cstdint>
#include <raylib.h>
#include <string>

class ScreenInfo {

public:
    Texture screen_tex;
    uint32_t width;
    uint32_t height;
    uint32_t up_width;
    uint32_t up_height;
};

namespace {

void init_screen_tex(Texture &screen_tex, std::uint32_t width,
                     std::uint32_t height)
{

    Image img = GenImageColor(width, height, BLACK);
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    screen_tex = LoadTextureFromImage(img);

    UnloadImage(img);
}

} // namespace

Screen::Screen(uint32_t width, uint32_t height, uint32_t upscaled_width,
               uint32_t upscaled_height, std::string name)
{
    this->info = new ScreenInfo;

    SetTraceLogLevel(LOG_ERROR);

    InitWindow(upscaled_width, upscaled_height, name.c_str());

    this->info->width = width;
    this->info->height = height;
    this->info->up_width = upscaled_width;
    this->info->up_height = upscaled_height;
    init_screen_tex(this->info->screen_tex, width, height);
}

Screen::~Screen()
{
    UnloadTexture(this->info->screen_tex);
    delete this->info;
    CloseWindow();
}

void Screen::update(const void *pixels)
{
    Rectangle src_rect = {0, (float)this->info->height,
                          (float)this->info->width, (float)this->info->height};
    Rectangle dest_rect = {0, 0, (float)this->info->up_width,
                           (float)this->info->up_height};

    UpdateTexture(this->info->screen_tex, pixels);

    BeginDrawing();
    DrawTexturePro(this->info->screen_tex, src_rect, dest_rect, {0.f, 0.f}, 0.f,
                   WHITE);
    EndDrawing();
}

bool Screen::should_close() const
{
    return WindowShouldClose();
}

#endif
