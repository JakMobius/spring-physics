#pragma once

#include "../../../world/geometry-object.hpp"
#include "../../../world/material.hpp"
#include "../physics/vertex.hpp"
#include "world-object.hpp"

class VertexObject : public WorldObject {

    std::unique_ptr<PhysicsVertex> m_physics_vertex = nullptr;
    GeometryObject* m_geometry_object = nullptr;
    Material* m_material = nullptr;

public:
    explicit VertexObject(World* world);
    ~VertexObject();

    PhysicsVertex* get_physics_vertex() { return m_physics_vertex.get(); }
    GeometryObject* get_geometry_object() { return m_geometry_object; }

    void tick(float dt) override;

    void create_colored_mesh(const Vec3f& color);
};