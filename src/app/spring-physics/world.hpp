#pragma once

#include "../../world/geometry-object.hpp"
#include "../../world/material.hpp"
#include "../renderer/rendering-context.hpp"
#include "objects/surface-triangle-object.hpp"
#include "objects/world-object.hpp"
#include "particle-host.hpp"
#include "physics/physics-engine.hpp"
#include <vector>

class World {
    std::unique_ptr<ConcurrentPhysicsEngine> m_physics_engine {};
    RenderingContext* m_ctx;
    std::unique_ptr<ParticleHost> m_particle_host {};
    std::unordered_set<WorldObject*> m_objects {};

    std::vector<std::unique_ptr<SurfaceTriangleObject>> m_surface_mesh {};

    GeometryObject* m_floor = nullptr;
    Material* m_floor_material = nullptr;

public:
    void add_floor();

    World(RenderingContext* drawer);

    ~World();

    std::vector<std::unique_ptr<SurfaceTriangleObject>>& get_surface_mesh();

    RenderingContext* get_rendering_context() { return m_ctx; }
    ConcurrentPhysicsEngine* get_physics_engine() { return m_physics_engine.get(); }
    ParticleHost* get_particle_host() { return m_particle_host.get(); }

    void set_drawer(RenderingContext* drawer) { m_ctx = drawer; }

    void tick(float dt);

    void add_object(WorldObject* object) { m_objects.insert(object); }
    void remove_object(WorldObject* object) { m_objects.erase(object); }
};