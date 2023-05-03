#pragma once

#include "../../../world/geometry-object.hpp"
#include "../../../world/material.hpp"
#include "../physics/spring.hpp"
#include "world-object.hpp"

class SpringObject : public WorldObject {
    std::unique_ptr<PhysicsSpring> m_physics_spring{};
    std::unique_ptr<GeometryObject> m_geometry_object = nullptr;
    std::unique_ptr<Material> m_material = nullptr;

  public:
    explicit SpringObject(World* world);
    ~SpringObject() override;

    PhysicsSpring* get_physics_spring() {
        return m_physics_spring.get();
    }
    GeometryObject* get_geometry_object() {
        return m_geometry_object.get();
    }

    void create_colored_mesh(const Vec3f& color);

    void tick(float dt) override;
};