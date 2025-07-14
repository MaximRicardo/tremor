#pragma once

#include "../renderer/triangle.hpp"
#include <filesystem>
#include <vector>

namespace ObjLoader {

std::vector<Triangle> load_file(const std::filesystem::path &path);

}
