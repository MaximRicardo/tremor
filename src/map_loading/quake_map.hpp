#pragma once

// for reading quake .map files
// NOTE: SPECIFICALLY USES THE VALVE FORMAT

#include "../map.hpp"
#include <filesystem>

namespace QuakeMapLoader {

Map load_file(const std::filesystem::path &path);

};
