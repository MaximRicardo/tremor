#include "obj_loader.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// reads a line prefixed with 'f'. the stream must start at the first num.
void read_idxs_line(std::vector<size_t> &idxs, std::stringstream &stream)
{
    size_t idx;
    while (stream >> idx) {
        idxs.push_back(idx);

        // we don't care about the rest rn
        char c;
        while (stream >> c) {
            if (c != '/') {
                stream.seekg(-1, std::ios::cur);
                break;
            }
            size_t num;
            stream >> num;
        }
    }
}

// triangulation not supported (yet)
std::vector<Triangle> convert_vs_to_tris(const std::vector<Vec3> &vs,
                                         const std::vector<size_t> &idxs)
{
    std::vector<Triangle> tris;

    for (size_t i = 0; i < idxs.size(); i += 3) {
        tris.emplace_back(vs[idxs[i] - 1], vs[idxs[i + 1] - 1],
                          vs[idxs[i + 2] - 1]);
    }

    return tris;
}

} // namespace

ObjLoader::ObjLoader(std::string f_path) : f_path(f_path)
{
    this->load();
}

void ObjLoader::read_file(std::ifstream &file)
{
    std::string line;

    std::vector<Vec3> vs;
    std::vector<size_t> idxs;

    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        std::string prefix = line.substr(0, line.find(" "));
        std::stringstream stream(line.substr(prefix.length()));

        if (prefix == "v") {
            float x, y, z;
            stream >> x;
            stream >> y;
            stream >> z;
            vs.emplace_back(x, y, z);
        } else if (prefix == "vt") {
            // i'll implement this when i add texture mapping
            continue;
        } else if (prefix == "f") {
            read_idxs_line(idxs, stream);
        }
    }

    this->tris = convert_vs_to_tris(vs, idxs);
}

void ObjLoader::load()
{
    std::ifstream file(this->f_path);
    if (file.fail()) {
        throw std::runtime_error("can't open obj file " + this->f_path + ": " +
                                 strerror(errno));
    }

    this->read_file(file);

    file.close();
}
