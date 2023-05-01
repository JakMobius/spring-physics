#pragma once

#include <vector>
#include "world-object.hpp"
#include "../physics/terrain-polygon.hpp"
#include "../../../world/geometry-object.hpp"
#include "../../../world/material.hpp"

class MapObject : public WorldObject {

    std::vector<TerrainPolygon*> m_physics_triangles {};
    GeometryObject *m_geometry_object = nullptr;
    Material* m_material = nullptr;

public:
    explicit MapObject(World *world, const std::string &path, const Matrix4f &transform);

    ~MapObject() override;

    GeometryObject *get_geometry_object() { return m_geometry_object; }

    void tick(float dt) override;

    void create_mesh(const std::string &path, const Matrix4f& transform);
};