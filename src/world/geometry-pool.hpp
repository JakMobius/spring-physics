#pragma once

#include "../app/renderer/scene-buffer.hpp"
#include "../utils/index-pool.hpp"
#include "../utils/vec2.hpp"
#include "geometry-object-config.hpp"
#include "geometry-object.hpp"
#include "material.hpp"
#include "scene-vertex.hpp"
#include <list>
#include <map>
#include <unordered_set>

class RenderingContext;

class GeometryPool {
    const int material_stride = 8;
    const int transform_stride = 16;

    std::vector<unsigned int> start_indices{};
    std::vector<unsigned int> size_array{};

    // TODO: It seems that these two identical field groups could be wrapped in some nice fancy structures

    IndexPool<int> m_matrix_buffer_index_pool{};
    std::unordered_set<GeometryObject*> m_dirty_transform_objects{};

    IndexPool<int> m_material_buffer_index_pool{};
    std::unordered_set<Material*> m_dirty_materials{};

    std::list<GeometryObject*> m_objects{};
    std::unordered_set<Material*> m_materials{};

    GeometryObject* m_defragmented_elements = nullptr;

    RenderingContext* m_drawer;

    void defragment_single_object(GeometryObject* object, int offset);

    static void extend_buffer(std::vector<float>& buffer, int min_capacity);
    int allocate_buffer(int size);
    int get_free_vertex_buffer_offset();

    void copy_geometry(int offset, const std::vector<SceneVertex>& vertices, int matrix_index);

    void insert_offsets_to_arrays(int free_index, int buffer_stride);
    void remove_offset_from_arrays(int offset);
    void adjust_offset_from_arrays(int offset, int new_offset);

    int create_matrix();
    void copy_matrix(int index, const Matrix4f& matrix);

  public:
    const std::vector<unsigned int>& get_start_indices() {
        return start_indices;
    };
    const std::vector<unsigned int>& get_size_array() {
        return size_array;
    };

    int get_object_count() {
        return m_objects.size();
    }

    GeometryPool(RenderingContext* drawer);

    std::unique_ptr<GeometryObject> create_object(const GeometryObjectConfig& object_config, GeometryObject* parent);
    void destroy_object(GeometryObject* object);

    SceneStorageBuffer* get_vertex_buffer();
    SceneStorageBuffer* get_material_buffer();
    SceneStorageBuffer* get_matrix_buffer();

    std::unique_ptr<Material> create_material();
    void destroy_material(Material* material);

    void synchronize();
    void defragment_buffer(int limit);

    void update_transforms();
    void update_materials();

    void update_material_delayed(Material* material);
    void update_transform_delayed(GeometryObject* object);

    void update_object_transform(GeometryObject* object);
    void update_material(Material* material);
};
