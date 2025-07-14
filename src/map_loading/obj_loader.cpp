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

constexpr size_t default_tex_idx = 0;

// vertex coordinates, vertex normals, texture coords and such are stored in
// this class
template <typename T> struct VertexAttrib {

    std::vector<T> values;
    std::vector<size_t> idxs;
};

// reads a line prefixed with 'f'. the stream must start at the first num.
void read_idxs_line(VertexAttrib<Vec3> &v_coords,
                    VertexAttrib<Vec2> &v_tex_coords,
                    VertexAttrib<Vec3> &v_normals, std::stringstream &stream)
{
    size_t idx;
    while (stream >> idx) {
        v_coords.idxs.push_back(idx);

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
                v_tex_coords.idxs.push_back(num);
            else if (i == 1)
                v_normals.idxs.push_back(num);

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
std::vector<Triangle> convert_v_coords_to_tris(VertexAttrib<Vec3> v_coords,
                                               VertexAttrib<Vec2> v_tex_coords,
                                               VertexAttrib<Vec3> v_normals)
{
    std::vector<Triangle> tris;

    for (size_t i = 0; i < v_coords.idxs.size(); i += 3) {
        Triangle tri({v_coords.values[v_coords.idxs[i] - 1],
                      v_coords.values[v_coords.idxs[i + 1] - 1],
                      v_coords.values[v_coords.idxs[i + 2] - 1]},
                     {v_tex_coords.values[v_tex_coords.idxs[i] - 1],
                      v_tex_coords.values[v_tex_coords.idxs[i + 1] - 1],
                      v_tex_coords.values[v_tex_coords.idxs[i + 2] - 1]},
                     default_tex_idx);

        // we're only checking against the first vertex normal cuz that
        // SHOULD be good enough. i think.
        match_normals(tri, v_normals.values[v_normals.idxs[i] - 1]);

        tris.push_back(tri);
    }

    return tris;
}

std::vector<Triangle> read_file(std::ifstream &file)
{
    std::string line;

    VertexAttrib<Vec3> v_coords;
    VertexAttrib<Vec2> v_tex_coords;
    VertexAttrib<Vec3> v_normals;

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
            v_coords.values.emplace_back(x, y, z);
        } else if (prefix == "vt") {
            float u, v;
            stream >> u;
            stream >> v;
            v_tex_coords.values.emplace_back(u, v);
        } else if (prefix == "vn") {
            float x, y, z;
            stream >> x;
            stream >> y;
            stream >> z;
            v_normals.values.emplace_back(x, y, z);
        } else if (prefix == "f") {
            read_idxs_line(v_coords, v_tex_coords, v_normals, stream);
        }
    }

    return convert_v_coords_to_tris(v_coords, v_tex_coords, v_normals);
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
