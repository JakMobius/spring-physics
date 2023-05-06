//
// Created by Артем on 04.02.2022.
//

#include "world.hpp"
#include "../../world/shape-generator.hpp"
#include "particle-host.hpp"

void World::tick(float dt) {
    for (auto& object : m_objects) {
        object->tick(dt);
    }
    m_particle_host->upload_particles(*m_ctx);
    m_particle_host->tick(dt);
}

void World::add_floor() {
    ShapeGenerator generator;

    m_floor_material = m_ctx->m_geometry_pool.get()->create_material();
    m_floor_material->set_color({0.15, 0.5, 0.2});
    m_floor_material->set_grid(true);

    float floor_level = m_physics_engine->get_floor_level();

    generator.add_cube({0, -0.07f + floor_level, 0}, {1000, 0.1, 1000}, m_floor_material.get());
    m_floor = m_ctx->m_geometry_pool.get()->create_object({generator.get_mesh()}, nullptr);
    generator.reset();
}

World::World(RenderingContext* drawer)
    : m_ctx(drawer) {
    m_physics_engine = std::make_unique<ConcurrentPhysicsEngine>();
    m_particle_host = std::make_unique<ParticleHost>();

    add_floor();
}

World::~World() {
    m_ctx->m_geometry_pool.get()->destroy_material(m_floor_material.get());
}
