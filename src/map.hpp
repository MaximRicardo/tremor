#pragma once

#include "bsp.hpp"
#include "camera.hpp"
#include "mat4x4.hpp"
#include "texture.hpp"
#include "triangle.hpp"
#include <span>
#include <vector>

class MapEntity {

public:
    BSP bsp;
    Vec3 pos;

    MapEntity(std::span<const Triangle> tris, Vec3 pos);

    Matrix4x4 get_transform() const;

    void render(std::span<Color> frame, std::span<float> depth_buffer,
                const Camera &cam, std::span<const Texture> texs) const;
};

class Map {

public:
    // worldspawn is always first
    std::vector<MapEntity> entities;

    void render(std::span<Color> frame, std::span<float> depth_buffer,
                const Camera &cam, std::span<const Texture> texs) const;

    size_t n_triangles() const;
};
