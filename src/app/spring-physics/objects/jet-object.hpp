#pragma once

#include "../../../world/geometry-object.hpp"
#include "../../../world/material.hpp"
#include "../physics/jet.hpp"
#include "../physics/vertex.hpp"
#include "world-object.hpp"

class JetObject : public WorldObject {
    std::unique_ptr<GeometryObject> m_geometry_object;
    std::unique_ptr<Material> m_material = nullptr;
    std::unique_ptr<PhysicsJet> m_physics_jet = nullptr;
    float m_max_force = 1.0f;
    float m_particles_accumulator = 0.0f;
    float m_particle_intensity = 300.0f; // Particles per second on max thrust
  public:
    JetObject(World* world, PhysicsVertex* head_vertex, PhysicsVertex* tail_vertex);

    ~JetObject() override;

    void set_max_force(float force);

    float get_max_force() const;

    void tick(float dt) override;

    void create_colored_mesh(const Vec3f& color);

    PhysicsJet* get_physics_jet();
};