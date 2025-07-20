#include "quake_map.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

void skip_char(std::ifstream &file, char expected_char)
{
    char c;
    file >> c;

    if (c != expected_char) {
        throw std::runtime_error(std::string("error: expected '") +
                                 expected_char + "', got '" + c + "'");
    }
}

// represents a key value pair in an entity
class EntityInfo {

public:
    std::string key;
    std::string value;

    explicit EntityInfo(std::ifstream &file);
};

EntityInfo::EntityInfo(std::ifstream &file)
{
    file >> this->key;
    file >> this->value;

    // remove the encasing double quotes
    this->key.erase(this->key.begin());
    this->key.erase(this->key.end() - 1);
    this->value.erase(this->value.begin());
    this->value.erase(this->value.end() - 1);
}

// each plane of each brush is represented by a triangle (and some other info)
class BrushPlane {

public:
    std::array<Vec3, 3> vs;
    std::string texture;
    Vec3 u, v;
    float rotation;
    Vec2 offset;
    Vec2 scale;

    explicit BrushPlane(std::ifstream &file);
};

BrushPlane::BrushPlane(std::ifstream &file)
{
    for (auto &v : this->vs) {
        skip_char(file, '(');
        file >> v.x;
        file >> v.y;
        file >> v.z;
        skip_char(file, ')');
    }

    file >> this->texture;

    skip_char(file, '[');
    file >> this->u.x;
    file >> this->u.y;
    file >> this->u.z;
    file >> this->offset.x;
    skip_char(file, ']');

    skip_char(file, '[');
    file >> this->v.x;
    file >> this->v.y;
    file >> this->v.z;
    file >> this->offset.y;
    skip_char(file, ']');

    file >> this->rotation;
    file >> this->scale.x;
    file >> this->scale.y;
}

class Brush {

public:
    std::vector<BrushPlane> planes;

    // file should be pointing to the line after the left curly marking the
    // start of the brush
    Brush(std::ifstream &file);
};

Brush::Brush(std::ifstream &file)
{
    while (true) {
        file >> std::ws;
        if (file.peek() == '}')
            break;

        this->planes.emplace_back(file);
    }

    skip_char(file, '}');
}

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
};

Entity::Entity(std::ifstream &file)
{
    while (true) {
        file >> std::ws;
        if (file.peek() == '}')
            break;

        if (file.peek() == '{') {
            skip_char(file, '{');
            this->brushes.emplace_back(file);
        } else {
            this->info.emplace_back(file);
        }
    }

    skip_char(file, '}');

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

std::vector<Triangle> read_file(std::ifstream &file)
{
    std::vector<Entity> entities;

    char c;
    while (true) {
        file >> std::ws;
        file >> c;
        if (c == EOF || file.eof())
            break;

        if (c == '}')
            throw std::runtime_error("error: extraneous '}'");
        else if (c == '{')
            entities.emplace_back(file);
    }

    for (const auto &entity : entities) {
        std::cout << "found entity named " << entity.name << "\n";
        for (const auto &brush : entity.brushes) {
            std::cout << "new brush\n";
            for (const auto &plane : brush.planes) {
                std::cout << "new plane\n";
                for (auto it = plane.vs.begin(); it < plane.vs.end(); ++it) {
                    std::cout << "v" << std::distance(plane.vs.begin(), it)
                              << " = (" << it->x << ", " << it->y << ", "
                              << it->z << ")\n";
                }
            }
        }
    }

    return {};
}

} // namespace

std::vector<Triangle>
QuakeMapLoader::load_file(const std::filesystem::path &path)
{
    std::ifstream file(path);
    if (file.fail()) {
        throw std::runtime_error("can't open quake map file " + path.string() +
                                 ": " + std::strerror(errno));
    }

    auto tris = read_file(file);

    return tris;
}
