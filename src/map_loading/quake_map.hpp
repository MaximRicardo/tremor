#pragma once

// for reading quake .map files
// NOTE: SPECIFICALLY USES THE VALVE FORMAT

#include "../triangle.hpp"
#include <filesystem>
#include <vector>

namespace QuakeMapLoader {

std::vector<Triangle> load_file(const std::filesystem::path &path);

};
