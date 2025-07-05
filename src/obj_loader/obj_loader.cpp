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
void read_idxs_line(std::vector<size_t> &vs_idxs, std::vector<size_t> &vts_idxs,
                    std::stringstream &stream)
{
    size_t idx;
    while (stream >> idx) {
        vs_idxs.push_back(idx);

        // there's probably a cleaner way to do this but this works
        char c;
        size_t i = 0;
        while (stream >> c) {
            if (c != '/') {
                stream.seekg(-1, std::ios::cur);
                break;
            }
            size_t num;
            stream >> num;

            if (i == 0)
                vts_idxs.push_back(num);

            ++i;
        }
    }
}

// triangulation not supported (yet)
std::vector<Triangle> convert_vs_to_tris(const std::vector<Vec3> &vs,
                                         const std::vector<size_t> &vs_idxs,
                                         const std::vector<Vec2> &vts,
                                         const std::vector<size_t> &vts_idxs)
{
    std::vector<Triangle> tris;

    for (size_t i = 0; i < vs_idxs.size(); i += 3) {
        Triangle tri({vs[vs_idxs[i] - 1], vs[vs_idxs[i + 1] - 1],
                      vs[vs_idxs[i + 2] - 1]},
                     {vts[vts_idxs[i] - 1], vts[vts_idxs[i + 1] - 1],
                      vts[vts_idxs[i + 2] - 1]});

        tris.push_back(tri);
    }

    return tris;
}

std::vector<Triangle> read_file(std::ifstream &file)
{
    std::string line;

    std::vector<Vec3> vs;
    std::vector<size_t> vs_idxs;
    std::vector<Vec2> vts;
    std::vector<size_t> vts_idxs;

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
            float u, v;
            stream >> u;
            stream >> v;
            vts.emplace_back(u, v);
        } else if (prefix == "f") {
            read_idxs_line(vs_idxs, vts_idxs, stream);
        }
    }

    return convert_vs_to_tris(vs, vs_idxs, vts, vts_idxs);
}

} // namespace

std::vector<Triangle> ObjLoader::load_file(std::string file_path)
{
    std::ifstream file(file_path);
    if (file.fail()) {
        throw std::runtime_error("can't open obj file " + file_path + ": " +
                                 strerror(errno));
    }

    auto tris = read_file(file);

    file.close();

    return tris;
}
