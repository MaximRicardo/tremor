#pragma once

#include "../renderer/triangle.hpp"
#include <string>
#include <vector>

namespace ObjLoader {

std::vector<Triangle> load_file(std::string file_path);

}
