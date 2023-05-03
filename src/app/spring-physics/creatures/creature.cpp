//
// Created by Артем on 04.02.2022.
//

#include "creature.hpp"
#include "../world.hpp"

Vec3f Creature::get_center() const {
    Vec3f center = {0, 0, 0};

    for (auto& vertex : m_vertices) {
        center += vertex->get_physics_vertex()->m_position;
    }

    center /= (float)m_vertices.size();

    return center;
}

Creature::Creature(World* world, const CreatureConfig& config)
    : WorldObject(world) {
    for (auto& vertex_config : config.m_vertices) {
        auto vertex = std::make_unique<VertexObject>(world);
        vertex_config.apply(vertex.get());
        m_vertices.push_back(std::move(vertex));
    }

    for (auto& spring_config : config.m_springs) {
        auto spring = std::make_unique<SpringObject>(world);
        auto physics_spring = spring->get_physics_spring();
        spring_config.apply(spring.get());

        physics_spring->m_vertex_a = m_vertices[spring_config.m_vertex_a_index]->get_physics_vertex();
        physics_spring->m_vertex_b = m_vertices[spring_config.m_vertex_b_index]->get_physics_vertex();
        physics_spring->m_target_length = physics_spring->get_length();

        m_springs.push_back(std::move(spring));
    }

    for (auto& muscle_config : config.m_muscles) {
        auto muscle = std::make_unique<Muscle>();
        muscle_config.apply(muscle.get(), m_springs[muscle_config.m_spring_index].get());
        m_muscles.push_back(std::move(muscle));
    }

    for (auto& surface_config : config.m_surfaces) {
        auto surface = std::make_unique<CreatureTriangleObject>(world,
                                                                m_vertices[surface_config.m_vertex_a_index]->get_physics_vertex(),
                                                                m_vertices[surface_config.m_vertex_b_index]->get_physics_vertex(),
                                                                m_vertices[surface_config.m_vertex_c_index]->get_physics_vertex());
        m_surfaces.push_back(std::move(surface));
    }

    for (auto& jet_config : config.m_jets) {
        auto jet = std::make_unique<JetObject>(world,
                                               m_vertices[jet_config.m_head_vertex_index]->get_physics_vertex(),
                                               m_vertices[jet_config.m_tail_vertex_index]->get_physics_vertex());
        jet->set_max_force(jet_config.m_max_force);
        m_jet_objects.push_back(std::move(jet));
    }

    m_world->get_physics_engine()->add_creature(this);
    m_world->add_object(this);
}

Creature::~Creature() {
    m_world->remove_object(this);
    m_world->get_physics_engine()->remove_creature(this);
}

void Creature::tick(float dt) {
}

void Creature::physics_tick() {
}
