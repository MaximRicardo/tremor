#pragma once

#include "../triangle.hpp"
#include <filesystem>
#include <vector>

namespace QuakeMapLoader {

std::vector<Triangle> load_file(const std::filesystem::path &path);

};
