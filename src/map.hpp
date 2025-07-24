#pragma once

#include "bsp.hpp"
#include "camera.hpp"
#include "frame.hpp"
#include "mat4x4.hpp"
#include "texture.hpp"
#include "triangle.hpp"
#include "vector/vec3.hpp"
#include <memory>
#include <span>
#include <string>
#include <vector>

class MapEntity {

public:
    // point entites don't have a bsp
    std::unique_ptr<BSP> bsp;
    Vec3 pos;
    std::string name;

    MapEntity(std::span<const Triangle> tris, Vec3 pos, std::string_view name);

    bool is_point_entity() const;
    Matrix4x4 get_transform() const;
    Matrix4x4 get_inv_transform() const;
    void render(Frame &frame, const Camera &cam,
                std::span<const Texture> texs) const;
};

class Map {

public:
    // worldspawn is always first
    std::vector<MapEntity> entities;
    std::vector<Texture> textures;

    // no constructor, MUAH HAH HAH HA

    void render(Frame &frame, const Camera &cam) const;
    Vec3 get_player_start() const;
    size_t n_triangles() const;
    const MapEntity *find_entity(std::string_view name) const;
    MapEntity *find_entity(std::string_view name);
};
