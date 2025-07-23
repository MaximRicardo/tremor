#pragma once

#include "../map.hpp"
#include <filesystem>
#include <string>

namespace QuakeMapLoader {

enum class Format {

    DETECT,
    QUAKE_1,
    VALVE,

};

std::string format_name(Format format);

Map load_file(const std::filesystem::path &path,
              Format format = Format::DETECT);

}; // namespace QuakeMapLoader
