// i might wanna make a seperate lexer pass if this gets too complex

#include "quake_map.hpp"
#include "../constants.hpp"
#include "../plane.hpp"
#include "../shape.hpp"
#include "../utils/string.hpp"
#include "wad.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <iterator>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

// idx of the worldspawn entity in the entity list
constexpr size_t worldspawn_idx = 0;

// each plane of each brush is represented by a triangle (and some other info)
class BrushPlane {

    // not necessary in the value format
    void get_u_v_dirs();

    // works for both quake and valve formats
    void get_vs(std::istringstream &stream);

    // constructor for the specific format
    void construct_quake(std::string_view line);
    void construct_valve(std::string_view line);

public:
    std::array<Vec3, 3> vs;
    std::string texture;
    Vec3 u, v;
    float rotation;
    Vec2 offset;
    Vec2 scale;

    BrushPlane(std::string_view line, QuakeMapLoader::Format format);

    Plane get_plane() const;
    Vec2 get_point_tex_coord(const Vec3 &p) const;
    size_t get_tex_idx(std::span<const Texture> textures) const;
};

class Brush {

    // finds the plane of the brush the given polygon is on
    const BrushPlane &poly_plane(const Polygon &poly) const;
    BrushPlane &poly_plane(const Polygon &poly);

    ConvexShape get_shape() const;

public:
    std::vector<BrushPlane> planes;

    Brush(std::ifstream &file, QuakeMapLoader::Format format);

    std::vector<Triangle> get_tris(std::span<const Texture> textures) const;
};

// represents a key value pair in an entity
class EntityInfo {

public:
    std::string key;
    std::string value;

    explicit EntityInfo(std::string_view line);
};

class Entity {

    void set_name();

public:
    std::string name;
    std::vector<EntityInfo> info;
    std::vector<Brush> brushes;

    // file should be pointing to the line after the left curly marking the
    // start of the entity
    // format would be changed if the class has a "mapversion" entry
    Entity(std::ifstream &file, QuakeMapLoader::Format &format);

    size_t get_info_idx(std::string_view key) const;
    MapEntity to_map_entity(std::span<const Texture> textures) const;
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

void BrushPlane::get_vs(std::istringstream &stream)
{
    for (auto &v : this->vs) {
        skip_char(stream, '(');
        // y and z are swapped in .map files
        stream >> v.x;
        stream >> v.z;
        stream >> v.y;
        skip_char(stream, ')');
    }
}

void BrushPlane::get_u_v_dirs()
{
    auto plane = this->get_plane();

    Vec3 up(0.f, 1.f, 0.f);
    Vec3 left(-1.f, 0.f, 0.f);

    Vec3 back(0.f, 0.f, 1.f);

    bool u_used_back = false;
    bool v_used_back = false;

    if (plane.normal.dot(up) < 1.f - Consts::epsilon) {
        this->u = up.cross(plane.normal);
    } else {
        this->u = back.cross(plane.normal);
        u_used_back = true;
    }

    if (plane.normal.dot(left) < 1.f - Consts::epsilon) {
        this->v = left.cross(plane.normal);
    } else {
        this->v = back.cross(plane.normal);
        v_used_back = true;
    }

    assert(!(u_used_back && v_used_back));
    // in case NDEBUG is defined
    (void)u_used_back;
    (void)v_used_back;
}

void BrushPlane::construct_quake(std::string_view line)
{
    std::istringstream stream((std::string(line)));

    this->get_vs(stream);

    stream >> this->texture;
    this->texture = String::str_tolower(this->texture);

    stream >> this->offset.x;
    stream >> this->offset.y;

    stream >> this->rotation;

    stream >> this->scale.x;
    stream >> this->scale.y;

    this->get_u_v_dirs();
}

void BrushPlane::construct_valve(std::string_view line)
{
    std::istringstream stream((std::string(line)));

    this->get_vs(stream);

    stream >> this->texture;
    this->texture = String::str_tolower(this->texture);

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

BrushPlane::BrushPlane(std::string_view line, QuakeMapLoader::Format format)
{
    switch (format) {

    case QuakeMapLoader::Format::QUAKE_1:
        this->construct_quake(line);
        break;

    case QuakeMapLoader::Format::VALVE_220:
        this->construct_valve(line);
        break;

    case QuakeMapLoader::Format::DETECT:
        assert(false);
    }
}

Plane BrushPlane::get_plane() const
{
    Vec3 n = (this->vs[1] - this->vs[0])
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

size_t BrushPlane::get_tex_idx(std::span<const Texture> textures) const
{
    for (auto it = textures.begin(); it < textures.end(); ++it) {
        if (it->name == this->texture)
            return std::distance(textures.begin(), it);
    }

    return SIZE_MAX;
}

Brush::Brush(std::ifstream &file, QuakeMapLoader::Format format)
{
    std::string line;
    while (std::getline(file, line)) {
        remove_leading_ws(line);

        if (line.empty() || line.starts_with("//"))
            continue;
        else if (line.starts_with("}"))
            break;
        else
            this->planes.emplace_back(line, format);
    }
}

const BrushPlane &Brush::poly_plane(const Polygon &poly) const
{
    float closest_dist = 10000.f;
    const BrushPlane *closest = nullptr;

    for (const auto &plane : this->planes) {
        auto p = plane.get_plane();

        float max_dist = 0.f;
        for (const auto &v : poly.vs) {
            float dist = std::abs(p.normal.dot(v) - p.d);
            max_dist = std::max(max_dist, dist);
        }

        if (max_dist < closest_dist) {
            closest_dist = max_dist;
            closest = &plane;
        }
    }

    assert(closest);
    return *closest;
}

BrushPlane &Brush::poly_plane(const Polygon &poly)
{
    return const_cast<BrushPlane &>(std::as_const(*this).poly_plane(poly));
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

std::vector<Triangle> Brush::get_tris(std::span<const Texture> textures) const
{
    std::vector<Triangle> tris;

    auto shape = this->get_shape();

    for (const auto &poly : shape.polys) {
        const BrushPlane &plane = this->poly_plane(poly);

        size_t tex_idx = plane.get_tex_idx(textures);
        if (tex_idx == SIZE_MAX)
            throw std::runtime_error("error: texture " + plane.texture +
                                     " doesn't exist.");

        auto poly_tris = poly.get_triangles();
        for (auto &tri : poly_tris) {
            for (size_t i = 0; i < tri.vs.size(); ++i) {
                tri.vts[i] = plane.get_point_tex_coord(tri.vs[i]);
            }
            tri.tex_idx = tex_idx;
        }

        tris.insert(tris.end(), poly_tris.begin(), poly_tris.end());
    }

    return tris;
}

QuakeMapLoader::Format map_version_to_format(std::string_view version)
{
    if (version == "220")
        return QuakeMapLoader::Format::VALVE_220;

    throw std::runtime_error("error: unsupported .map version '" +
                             std::string(version) + "'.");
}

QuakeMapLoader::Format change_format(QuakeMapLoader::Format format,
                                     std::string_view new_fmt_name,
                                     bool should_update_fmt)
{
    QuakeMapLoader::Format ret = format;
    QuakeMapLoader::Format new_fmt = map_version_to_format(new_fmt_name);

    if (should_update_fmt) {
        ret = new_fmt;
    } else {
        if (new_fmt != format) {
            std::cerr << "warning: .map version mismatch. version '" +
                             QuakeMapLoader::format_name(new_fmt) +
                             "' specified in a file with version '" +
                             QuakeMapLoader::format_name(format) + "'\n";
        }
    }

    return ret;
}

Entity::Entity(std::ifstream &file, QuakeMapLoader::Format &format)
{
    bool detect_fmt = format == QuakeMapLoader::Format::DETECT;
    if (detect_fmt) {
        // assume standard quake until proven otherwise
        format = QuakeMapLoader::Format::QUAKE_1;
    }

    std::string line;
    while (std::getline(file, line)) {
        remove_leading_ws(line);

        if (line.empty() || line.starts_with("//"))
            continue;
        else if (line.starts_with("}"))
            break;
        else if (line.starts_with("{"))
            this->brushes.emplace_back(file, format);
        else {
            this->info.emplace_back(line);
            if (this->info.back().key == "mapversion") {
                format =
                    change_format(format, this->info.back().value, detect_fmt);
            }
        }
    }

    this->set_name();
}

void Entity::set_name()
{
    size_t name_idx = this->get_info_idx("classname");
    if (name_idx == SIZE_MAX) {
        // have fun finding out where said entity is lol
        throw std::runtime_error("error: entity missing a name");
    }

    this->name = this->info[name_idx].value;
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

MapEntity Entity::to_map_entity(std::span<const Texture> textures) const
{
    std::vector<Triangle> tris;

    for (const auto &brush : this->brushes) {
        auto brush_tris = brush.get_tris(textures);
        tris.insert(tris.end(), brush_tris.begin(), brush_tris.end());
    }

    return MapEntity(tris, this->get_pos());
}

// if the entity doesn't have an origin, the pos defaults to 0
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

void put_worldspawn_at_idx(std::vector<Entity> &entities, size_t idx)
{
    assert(worldspawn_idx < entities.size());

    for (auto &entity : entities) {
        if (entity.name != "worldspawn")
            continue;

        // btw, if worldspawn happens to already be at idx, swapping
        // entity with itself will just leave the value unchanged.
        std::swap(entity, entities[idx]);
        return;
    }

    throw std::runtime_error("error: missing the worldspawn entity.");
}

std::vector<Entity> get_entities(std::ifstream &file,
                                 QuakeMapLoader::Format format)
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
            entities.emplace_back(file, format);
    }

    put_worldspawn_at_idx(entities, worldspawn_idx);

    return entities;
}

std::vector<Texture> load_wad_file(const Entity &worldspawn,
                                   std::filesystem::path map_dir)
{
    std::filesystem::path wad_path =
        worldspawn.info[worldspawn.get_info_idx("wad")].value;

    try {
        return WAD::load_file(map_dir / wad_path);
    } catch (std::runtime_error &e) {
        throw std::runtime_error("failed to read WAD file '" +
                                 wad_path.string() + "': " + e.what());
    }
}

Map read_file(std::ifstream &file, const std::filesystem::path &path,
              QuakeMapLoader::Format format)
{
    std::filesystem::path dir = path;
    dir.remove_filename();

    auto entities = get_entities(file, format);
    if (entities.empty())
        throw std::runtime_error("error: .map file '" + path.string() +
                                 "' is empty.");

    Map map;
    map.textures = load_wad_file(entities[worldspawn_idx], dir);

    for (const auto &entity : entities) {
        if (entity.brushes.empty())
            continue;
        map.entities.push_back(entity.to_map_entity(map.textures));
    }

    return map;
}

} // namespace

std::string QuakeMapLoader::format_name(Format format)
{
    switch (format) {

    case QuakeMapLoader::Format::DETECT:
        return "detect_version";

    case QuakeMapLoader::Format::QUAKE_1:
        return "quake 1";

    case QuakeMapLoader::Format::VALVE_220:
        return "valve 220";
    }
}

Map QuakeMapLoader::load_file(const std::filesystem::path &path, Format format)
{
    std::ifstream file(path);
    if (file.fail()) {
        throw std::runtime_error("can't open quake map file " + path.string() +
                                 ": " + std::strerror(errno));
    }

    return read_file(file, path, format);
}
