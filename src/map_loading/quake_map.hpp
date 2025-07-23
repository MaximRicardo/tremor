#pragma once

#include "../map.hpp"
#include <filesystem>

namespace QuakeMapLoader {

enum class Format {

    QUAKE_1,
    VALVE,

};

Map load_file(const std::filesystem::path &path, Format format);

}; // namespace QuakeMapLoader
