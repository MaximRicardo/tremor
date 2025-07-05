#pragma once

#include "../renderer/triangle.hpp"
#include <fstream>
#include <string>
#include <vector>

class ObjLoader {

    void read_file(std::ifstream &file);
    void load();

public:
    std::vector<Triangle> tris;
    std::string f_path;

    ObjLoader(std::string f_path);
};
