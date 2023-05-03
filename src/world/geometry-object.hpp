#pragma once

class GeometryPool;

#include "../utils/matrix4.hpp"
#include <vector>

class GeometryObject {
  public:
    GeometryObject(GeometryPool* pool,
                   int vertex_buffer_offset,
                   int vertex_buffer_length,
                   int matrix_buffer_index,
                   GeometryObject* parent);

    ~GeometryObject();

    void set_needs_transform_update();

    void update_transform();
    void update_subsidiary_transformations();

    GeometryPool* get_geometry_pool() const {
        return m_geometry_pool;
    }
    void set_geometry_pool(GeometryPool* pool) {
        m_geometry_pool = pool;
    }

    int get_vertex_buffer_offset() const {
        return m_vertex_buffer_offset;
    }
    void set_vertex_buffer_offset(int offset) {
        m_vertex_buffer_offset = offset;
    }
    int get_vertex_buffer_length() const {
        return m_vertex_buffer_length;
    }
    void set_vertex_buffer_length(int length) {
        m_vertex_buffer_length = length;
    }

    int get_matrix_buffer_index() const {
        return m_matrix_buffer_index;
    }
    void set_matrix_buffer_index(int index) {
        m_matrix_buffer_index = index;
    }

    GeometryObject* get_parent() const {
        return m_parent;
    }
    void set_parent(GeometryObject* parent) {
        m_parent = parent;
    }

    bool get_needs_transform_update() const {
        return m_needs_transform_update;
    }
    void set_needs_transform_update(bool needs_update) {
        m_needs_transform_update = needs_update;
    }

    GeometryObject* get_next() const {
        return m_next;
    }
    void set_next(GeometryObject* next) {
        m_next = next;
    }
    GeometryObject* get_prev() const {
        return m_prev;
    }
    void set_prev(GeometryObject* prev) {
        m_prev = prev;
    }

    std::vector<GeometryObject*>& get_children() {
        return m_children;
    }
    const std::vector<GeometryObject*>& get_children() const {
        return m_children;
    }

    const Matrix4f& get_transform() const {
        return m_transform;
    }
    void set_transform(const Matrix4f& transform);
    const Matrix4f& get_world_transform() const {
        return m_world_transform;
    }
    void set_world_transform(const Matrix4f& world_transform) {
        m_world_transform = world_transform;
    }

  private:
    GeometryPool* m_geometry_pool;

    int m_vertex_buffer_offset;
    int m_vertex_buffer_length;
    int m_matrix_buffer_index;
    GeometryObject* m_parent = nullptr;

    bool m_needs_transform_update = false;

    GeometryObject* m_next = nullptr;
    GeometryObject* m_prev = nullptr;

    std::vector<GeometryObject*> m_children{};

    Matrix4f m_transform{};
    Matrix4f m_world_transform{};
};