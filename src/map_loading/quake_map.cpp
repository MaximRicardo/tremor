// i might wanna make a seperate lexer pass if this gets too complex

#include "quake_map.hpp"
#include "../constants.hpp"
#include "../plane.hpp"
#include "../shape.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <istream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

// each plane of each brush is represented by a triangle (and some other info)
class BrushPlane {

public:
    std::array<Vec3, 3> vs;
    std::string texture;
    Vec3 u, v;
    float rotation;
    Vec2 offset;
    Vec2 scale;

    explicit BrushPlane(std::string_view line);

    Plane get_plane() const;
    Vec2 get_point_tex_coord(const Vec3 &p) const;
};

class Brush {

    // finds the plane of the brush the given polygon is on
    const BrushPlane *poly_plane(const Polygon &poly) const;
    BrushPlane *poly_plane(const Polygon &poly);

    ConvexShape get_shape() const;

public:
    std::vector<BrushPlane> planes;

    Brush(std::ifstream &file);

    std::vector<Triangle> get_tris() const;
};

// represents a key value pair in an entity
class EntityInfo {

public:
    std::string key;
    std::string value;

    explicit EntityInfo(std::string_view line);
};

class Entity {

public:
    std::string name;
    // classname is not included in this list
    std::vector<EntityInfo> info;
    std::vector<Brush> brushes;

    // file should be pointing to the line after the left curly marking the
    // start of the entity
    explicit Entity(std::ifstream &file);

    size_t get_info_idx(std::string_view key) const;
    MapEntity to_map_entity() const;
    Vec3 get_pos() const;
};

std::string whitespace = " \t";

void remove_leading_ws(std::string &str)
{
    if (str.empty())
        return;

    auto pos = str.find_first_not_of(whitespace);
    if (pos == std::string::npos)
        str = "";
    else
        str = str.substr(pos);
}

void skip_char(std::istringstream &stream, char expected_char)
{
    char c;
    stream >> c;

    if (c != expected_char) {
        throw std::runtime_error(std::string("error: expected '") +
                                 expected_char + "', got '" + c + "'");
    }
}

EntityInfo::EntityInfo(std::string_view line)
{
    std::istringstream stream((std::string(line)));

    stream >> std::quoted(this->key);
    stream >> std::quoted(this->value);
}

BrushPlane::BrushPlane(std::string_view line)
{
    std::istringstream stream((std::string(line)));

    for (auto &v : this->vs) {
        skip_char(stream, '(');
        stream >> v.x;
        stream >> v.y;
        stream >> v.z;
        skip_char(stream, ')');
    }

    stream >> this->texture;

    skip_char(stream, '[');
    stream >> this->u.x;
    stream >> this->u.y;
    stream >> this->u.z;
    stream >> this->offset.x;
    skip_char(stream, ']');

    skip_char(stream, '[');
    stream >> this->v.x;
    stream >> this->v.y;
    stream >> this->v.z;
    stream >> this->offset.y;
    skip_char(stream, ']');

    stream >> this->rotation;
    stream >> this->scale.x;
    stream >> this->scale.y;
}

Plane BrushPlane::get_plane() const
{
    Vec3 n = -(this->vs[1] - this->vs[0])
                  .cross(this->vs[2] - this->vs[0])
                  .normalize();
    float d = n.dot(this->vs[0]);

    return Plane(n, d);
}

Vec2 BrushPlane::get_point_tex_coord(const Vec3 &p) const
{
    Vec3 axis_u = this->u / this->scale.x;
    Vec3 axis_v = this->v / this->scale.y;

    Vec2 tex(p.dot(axis_u), p.dot(axis_v));
    tex += offset;

    // in quake, texture coordinates seems to have gone from -16 to 16 instead
    // of 0 to 1?
    tex += Vec2(16.f, 16.f);
    tex /= 2.f;
    tex /= 16.f;

    // correct for the flipped x and y too
    tex.x = 1.f - tex.x;
    tex.y = 1.f - tex.y;

    return tex;
}

Brush::Brush(std::ifstream &file)
{
    std::string line;
    while (std::getline(file, line)) {
        remove_leading_ws(line);

        if (line.empty() || line.starts_with("//"))
            continue;
        else if (line.starts_with("}"))
            break;
        else
            this->planes.emplace_back(line);
    }
}

const BrushPlane *Brush::poly_plane(const Polygon &poly) const
{
    for (const auto &plane : this->planes) {
        if (poly.get_plane().is_coplanar(plane.get_plane()))
            return &plane;
    }

    return nullptr;
}

BrushPlane *Brush::poly_plane(const Polygon &poly)
{
    return const_cast<BrushPlane *>(std::as_const(*this).poly_plane(poly));
}

ConvexShape Brush::get_shape() const
{
    ConvexShape shape = ConvexShape::box(
        Vec3(Consts::map_bounding_box_max_x - Consts::map_bounding_box_min_x,
             Consts::map_bounding_box_max_y - Consts::map_bounding_box_min_y,
             Consts::map_bounding_box_max_z - Consts::map_bounding_box_min_z));

    for (const auto &plane : this->planes) {
        shape.clip(plane.get_plane().flipped());
    }

    return shape;
}

std::vector<Triangle> Brush::get_tris() const
{
    std::vector<Triangle> tris;

    auto shape = this->get_shape();

    for (const auto &poly : shape.polys) {
        const BrushPlane *plane = this->poly_plane(poly);
        assert(plane);

        auto poly_tris = poly.get_triangles();
        for (auto &tri : poly_tris) {
            for (size_t i = 0; i < tri.vs.size(); ++i) {
                tri.vts[i] = plane->get_point_tex_coord(tri.vs[i]);
            }
        }

        tris.insert(tris.end(), poly_tris.begin(), poly_tris.end());
    }

    return tris;
}

Entity::Entity(std::ifstream &file)
{
    std::string line;
    while (std::getline(file, line)) {
        remove_leading_ws(line);

        if (line.empty() || line.starts_with("//"))
            continue;
        else if (line.starts_with("}"))
            break;
        else if (line.starts_with("{"))
            this->brushes.emplace_back(file);
        else
            this->info.emplace_back(line);
    }

    size_t name_idx = this->get_info_idx("classname");
    if (name_idx == SIZE_MAX) {
        // have fun finding out where said entity is lol
        throw std::runtime_error("error: entity missing a class name");
    }

    this->name = this->info[name_idx].value;
    this->info.erase(this->info.begin() + name_idx);
}

size_t Entity::get_info_idx(std::string_view key) const
{
    for (auto it = this->info.begin(); it < this->info.end(); ++it) {
        if (it->key != key)
            continue;

        return std::distance(this->info.begin(), it);
    }

    return SIZE_MAX;
}

MapEntity Entity::to_map_entity() const
{
    std::vector<Triangle> tris;

    for (const auto &brush : this->brushes) {
        auto brush_tris = brush.get_tris();
        tris.insert(tris.end(), brush_tris.begin(), brush_tris.end());
    }

    return MapEntity(tris, this->get_pos());
}

Vec3 Entity::get_pos() const
{
    size_t idx = this->get_info_idx("origin");
    if (idx == SIZE_MAX)
        return Vec3::zero();

    std::stringstream s(this->info[idx].value);

    Vec3 pos;
    s >> pos.x;
    s >> pos.y;
    s >> pos.z;
    return pos;
}

Map read_file(std::ifstream &file)
{
    std::vector<Entity> entities;

    std::string line;
    while (std::getline(file, line)) {
        remove_leading_ws(line);

        if (line.starts_with("//"))
            continue;
        else if (line.starts_with("}"))
            throw std::runtime_error("error: extraneous '}'");
        else if (line.starts_with("{"))
            entities.emplace_back(file);
    }

    Map map;

    for (const auto &entity : entities) {
        if (entity.brushes.empty())
            continue;
        map.entities.push_back(entity.to_map_entity());
    }

    return map;
}

} // namespace

Map QuakeMapLoader::load_file(const std::filesystem::path &path)
{
    std::ifstream file(path);
    if (file.fail()) {
        throw std::runtime_error("can't open quake map file " + path.string() +
                                 ": " + std::strerror(errno));
    }

    return read_file(file);
}
