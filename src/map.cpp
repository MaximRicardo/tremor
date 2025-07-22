#include "map.hpp"

MapEntity::MapEntity(std::span<const Triangle> tris, Vec3 pos)
    : bsp(tris), pos(pos)
{}

Matrix4x4 MapEntity::get_transform() const
{
    Matrix4x4 mat = Matrix4x4::identity();
    mat *= Matrix4x4::translate(this->pos);

    return mat;
}

Matrix4x4 MapEntity::get_inv_transform() const
{
    Matrix4x4 mat = Matrix4x4::identity();
    mat *= Matrix4x4::translate(-this->pos);

    return mat;
}

void MapEntity::render(std::span<Color> frame, std::span<float> depth_buffer,
                       const Camera &cam, std::span<const Texture> texs) const
{
    this->bsp.render(*this, frame, depth_buffer, cam, texs);
}

void Map::render(std::span<Color> frame, std::span<float> depth_buffer,
                 const Camera &cam) const
{
    for (auto &entity : this->entities) {
        entity.render(frame, depth_buffer, cam, this->textures);
    }
}

size_t Map::n_triangles() const
{
    size_t n = 0;

    for (const auto &entity : this->entities) {
        n += entity.bsp.n_triangles();
    }

    return n;
}
