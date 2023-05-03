
#include "geometry-object.hpp"
#include "geometry-pool.hpp"

void GeometryObject::update_transform() {
    if (m_parent) {
        if (m_parent->m_needs_transform_update)
            m_parent->update_transform();

        m_world_transform = m_parent->m_world_transform * m_transform;
    } else {
        m_world_transform = m_transform;
    }

    m_geometry_pool->update_object_transform(this);
    m_needs_transform_update = false;
}

void GeometryObject::set_needs_transform_update() {
    if (m_needs_transform_update)
        return;

    if (!m_parent || !m_parent->m_needs_transform_update) {
        m_geometry_pool->update_transform_delayed(this);
    }

    m_needs_transform_update = true;
    for (auto& child : m_children) {
        child->set_needs_transform_update();
    }
}

GeometryObject::GeometryObject(GeometryPool* pool, int vertex_buffer_offset,
                               int vertex_buffer_length, int matrix_buffer_index,
                               GeometryObject* parent)
    : m_geometry_pool(pool),
      m_vertex_buffer_offset(vertex_buffer_offset),
      m_vertex_buffer_length(vertex_buffer_length),
      m_matrix_buffer_index(matrix_buffer_index),
      m_parent(parent) {

    if (parent) {
        parent->m_children.push_back(this);
    }
}

void GeometryObject::set_transform(const Matrix4f& matrix) {
    m_transform = matrix;
    set_needs_transform_update();
}

void GeometryObject::update_subsidiary_transformations() {
    for (auto& child : m_children) {
        child->m_world_transform = m_world_transform * child->m_transform;
        m_geometry_pool->update_object_transform(child);
        child->m_needs_transform_update = false;
        child->update_subsidiary_transformations();
    }
}

GeometryObject::~GeometryObject() {
    if (m_geometry_pool) {
        m_geometry_pool->destroy_object(this);
    }
}
