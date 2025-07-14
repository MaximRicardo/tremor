#pragma once

#include "../renderer/triangle.hpp"
#include <string>
#include <vector>

namespace ObjLoader {

std::vector<Triangle> load_file(const std::string &file_path);

}
