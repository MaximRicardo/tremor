#include "map.hpp"
#include "polygon.hpp"
#include <string>
#include <string_view>
#include <utility>

MapEntity::MapEntity(std::span<const RenderPolygon> polys, Vec3 pos,
                     std::string_view name)
    : bsp(polys.empty() ? nullptr : new BSPTree(polys)), pos(pos),
      name(std::string(name))
{}

bool MapEntity::is_point_entity() const
{
    return bsp == nullptr;
}

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

void MapEntity::render(Frame &frame, const Camera &cam,
                       std::span<const Texture> texs) const
{
    if (is_point_entity())
        return;
    this->bsp->render(*this, frame, cam, texs);
}

void Map::render(Frame &frame, const Camera &cam) const
{
    for (auto &entity : this->entities) {
        entity.render(frame, cam, this->textures);
    }
}

Vec3 Map::get_player_start() const
{
    const MapEntity *entity = this->find_entity("info_player_start");
    if (!entity)
        return Vec3::zero();
    else
        return entity->pos;
}

size_t Map::n_triangles() const
{
    size_t n = 0;

    for (const auto &entity : this->entities) {
        if (entity.is_point_entity())
            continue;
        n += entity.bsp->get_root().n_triangles();
    }

    return n;
}

const MapEntity *Map::find_entity(std::string_view name) const
{
    for (auto &entity : this->entities) {
        if (entity.name == name)
            return &entity;
    }

    return nullptr;
}

MapEntity *Map::find_entity(std::string_view name)
{
    return const_cast<MapEntity *>(std::as_const(*this).find_entity(name));
}
