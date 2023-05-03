#pragma once

#include "../../../world/geometry-object.hpp"
#include "../../../world/material.hpp"
#include "../physics/terrain-polygon.hpp"
#include "world-object.hpp"
#include <vector>

class MapObject : public WorldObject {

    std::vector<std::unique_ptr<TerrainPolygon>> m_physics_triangles{};
    std::unique_ptr<GeometryObject> m_geometry_object = nullptr;
    std::unique_ptr<Material> m_material = nullptr;

  public:
    explicit MapObject(World* world, const std::string& path, const Matrix4f& transform);

    ~MapObject() override;

    GeometryObject* get_geometry_object() {
        return m_geometry_object.get();
    }

    void tick(float dt) override;

    void create_mesh(const std::string& path, const Matrix4f& transform);
};