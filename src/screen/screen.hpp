#pragma once

// completely abstracts away the underlying rendering library to prevent
// annoying shit like name collisions with the rendering library's namespace.

#include <cstdint>
#include <string>

class ScreenInfo;

class Screen {

    ScreenInfo *info;

public:
    Screen(uint32_t width, uint32_t height, uint32_t upscaled_width,
           uint32_t upscaled_height, std::string name);
    ~Screen();

    // pixels is in an R8G8B8A8 format
    void update(const void *pixels);

    bool should_close() const;
};
