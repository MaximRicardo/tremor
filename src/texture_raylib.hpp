#pragma once

// i'm gonna make my own PNG loader at some point, but rn i gotta get the
// renderer working first so i'ma just cheat and use raylib for image loading

#include <cstdint>
#include <string>
#include <vector>

namespace TextureRaylib {

struct LoadImageRet {

    // in R8B8G8 format
    std::vector<uint8_t> data;
    unsigned width, height;
};

LoadImageRet load_image(std::string path);

} // namespace TextureRaylib
