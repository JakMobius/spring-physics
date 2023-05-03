//
// Created by Артем on 04.02.2022.
//

#include "creature-triangle-object.hpp"
#include "../../../world/shape-generator.hpp"
#include "../world.hpp"

CreatureTriangleObject::CreatureTriangleObject(World* world, PhysicsVertex* vertex_a, PhysicsVertex* vertex_b,
                                               PhysicsVertex* vertex_c)
    : WorldObject(world) {
    m_physics_triangle = std::make_unique<PhysicsSurface>(vertex_a,
                                                          vertex_b,
                                                          vertex_c);
    m_world->add_object(this);
}

CreatureTriangleObject::~CreatureTriangleObject() {
    m_world->remove_object(this);
}

void CreatureTriangleObject::create_colored_mesh(const Vec3f& color) {
    auto geometry_pool = m_world->get_rendering_context()->m_geometry_pool.get();

    if (!m_material)
        m_material = geometry_pool->create_material();
    m_material->set_color(color);

    ShapeGenerator generator;
    // Add two triangles to deal with culling
    // I could also use a different culling mode, but it would
    // be more expensive
    generator.add_triangle({0, 0, 0}, {0, 1, 0}, {0, 0, 1}, m_material.get());
    generator.add_triangle({0, 0, 0}, {0, 0, 1}, {0, 1, 0}, m_material.get());

    m_geometry_object = geometry_pool->create_object({generator.get_mesh()}, nullptr);
}

void CreatureTriangleObject::tick(float dt) {
    if (!m_geometry_object)
        return;

    // The triangle points are:
    // {0, 0, 0}, {0, 1, 0}, {0, 0, 1}

    // The true triangle points are:
    // m_vertex_a->m_position, m_vertex_b->m_position, m_vertex_c->m_position

    Vec3f origin = m_physics_triangle->m_vertex_a->m_position;
    Vec3f point_b = m_physics_triangle->m_vertex_b->m_position - origin;
    Vec3f point_c = m_physics_triangle->m_vertex_c->m_position - origin;

    Vec3f basis_b = point_b;
    Vec3f basis_c = point_c;
    Vec3f basis_a = basis_b.cross(basis_c);

    Matrix4f transform({
        basis_a.x,
        basis_a.y,
        basis_a.z,
        0,
        basis_b.x,
        basis_b.y,
        basis_b.z,
        0,
        basis_c.x,
        basis_c.y,
        basis_c.z,
        0,
        origin.x,
        origin.y,
        origin.z,
        1,
    });

    m_geometry_object->set_transform(transform);
}
