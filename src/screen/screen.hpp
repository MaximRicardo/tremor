#pragma once

// completely abstracts away the underlying rendering library to prevent
// annoying shit like name collisions with the rendering library's namespace.

#include <string>

class ScreenInfo;

class Screen {

    ScreenInfo *info;

public:
    Screen(std::string name);
    ~Screen();

    // pixels is in an R8G8B8A8 format
    void update(const void *pixels);
    // must be called whenever the resolution is changed
    void update_resolution();

    bool should_close() const;
};
