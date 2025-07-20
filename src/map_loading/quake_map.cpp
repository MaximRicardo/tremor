#include "quake_map.hpp"
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

class Entity {

public:
    std::string name;
    // classname is not included in this list
    std::vector<EntityInfo> info;

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

        EntityInfo new_info(file);
        this->info.push_back(new_info);
    }

    size_t name_idx = this->get_info_idx("classname");
    if (name_idx == SIZE_MAX) {
        // have fun finding out where said entity is lol
        throw std::runtime_error("error: entity missing a class name\n");
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
        file >> c;
        if (c == EOF || file.eof())
            break;

        if (c == '{')
            entities.emplace_back(file);
    }

    /*
    for (const auto &entity : entities) {
        std::cout << "found entity named " << entity.name << "\n";
    }
    */

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
