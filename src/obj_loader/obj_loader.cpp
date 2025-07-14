#include "obj_loader.hpp"
#include "../renderer/plane.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

// reads a line prefixed with 'f'. the stream must start at the first num.
void read_idxs_line(std::vector<size_t> &vs_idxs, std::vector<size_t> &vts_idxs,
                    std::vector<size_t> &normal_idxs, std::stringstream &stream)
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
            else if (i == 1)
                normal_idxs.push_back(num);

            ++i;
        }
    }
}

static void match_normals(Triangle &tri, const Vec3 &normal)
{
    auto plane = tri.get_plane();
    if (plane.normal.dot(normal) < 0.f) {
        std::swap(tri.vs[1], tri.vs[2]);
        std::swap(tri.vts[1], tri.vts[2]);
    }
}

// triangulation not supported (yet)
std::vector<Triangle> convert_vs_to_tris(std::span<const Vec3> vs,
                                         std::span<const size_t> vs_idxs,
                                         std::span<const Vec2> vts,
                                         std::span<const size_t> vts_idxs,
                                         std::span<const Vec3> normals,
                                         std::span<const size_t> normal_idxs)
{
    std::vector<Triangle> tris;

    for (size_t i = 0; i < vs_idxs.size(); i += 3) {
        Triangle tri({vs[vs_idxs[i] - 1], vs[vs_idxs[i + 1] - 1],
                      vs[vs_idxs[i + 2] - 1]},
                     {vts[vts_idxs[i] - 1], vts[vts_idxs[i + 1] - 1],
                      vts[vts_idxs[i + 2] - 1]});

        // we're only checking against the first vertex normal cuz that
        // SHOULD be good enough. i think.
        match_normals(tri, normals[normal_idxs[i] - 1]);

        tris.push_back(tri);
    }

    return tris;
}

std::vector<Triangle> read_file(std::ifstream &file)
{
    std::string line;

    // vertices
    std::vector<Vec3> vs;
    std::vector<size_t> vs_idxs;
    // tex coords
    std::vector<Vec2> vts;
    std::vector<size_t> vts_idxs;
    // vectex normals
    std::vector<Vec3> normals;
    std::vector<size_t> normal_idxs;

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
        } else if (prefix == "vn") {
            float x, y, z;
            stream >> x;
            stream >> y;
            stream >> z;
            normals.emplace_back(x, y, z);
        } else if (prefix == "f") {
            read_idxs_line(vs_idxs, vts_idxs, normal_idxs, stream);
        }
    }

    return convert_vs_to_tris(vs, vs_idxs, vts, vts_idxs, normals, normal_idxs);
}

} // namespace

std::vector<Triangle> ObjLoader::load_file(const std::filesystem::path &path)
{
    std::ifstream file(path);
    if (file.fail()) {
        throw std::runtime_error("can't open obj file " + path.string() + ": " +
                                 strerror(errno));
    }

    auto tris = read_file(file);

    file.close();

    return tris;
}
