
#include "geometry-pool.hpp"
#include "../app/renderer/rendering-context.hpp"

GeometryPool::GeometryPool(RenderingContext* drawer)
    : m_drawer(drawer) {
}

void GeometryPool::defragment_single_object(GeometryObject* object, int new_position) {
    assert(new_position % SceneVertex::length == 0);

    auto& storage = get_vertex_buffer()->get_storage();

    auto old_position = object->get_vertex_buffer_offset();
    if (old_position == new_position)
        return;

    auto object_buffer_length = object->get_vertex_buffer_length();

    object->set_vertex_buffer_offset(new_position);

    adjust_offset_from_arrays(old_position, new_position);
    memmove(&storage[new_position], &storage[old_position],
            object_buffer_length * sizeof(float));

    get_vertex_buffer()->mark_dirty(new_position, new_position + object_buffer_length);
}

void GeometryPool::defragment_buffer(int limit) {
    if (m_defragmented_elements == nullptr)
        return;

    GeometryObject* last_defragmented = m_defragmented_elements;
    m_defragmented_elements = m_defragmented_elements->get_next();

    int free_offset = 0;

    auto prev = last_defragmented->get_prev();

    if (prev) {
        free_offset = prev->get_vertex_buffer_length() + prev->get_vertex_buffer_offset();
    }

    if (last_defragmented->get_vertex_buffer_offset() > free_offset) {
        defragment_single_object(last_defragmented, free_offset);
        if (!(--limit))
            return;
    }

    free_offset = last_defragmented->get_vertex_buffer_offset() +
                  last_defragmented->get_vertex_buffer_length();

    for (; m_defragmented_elements; m_defragmented_elements = m_defragmented_elements->get_next()) {
        GeometryObject* next = m_defragmented_elements;

        if (next->get_vertex_buffer_offset() > free_offset) {
            defragment_single_object(next, free_offset);
            if (!(--limit))
                return;
        }

        free_offset = next->get_vertex_buffer_offset() + next->get_vertex_buffer_length();
    }
}

void GeometryPool::extend_buffer(std::vector<float>& buffer, int min_capacity) {
    auto capacity = buffer.size();
    if (capacity >= min_capacity)
        return;
    if (capacity == 0)
        capacity = 128;

    while (capacity < min_capacity) {
        capacity *= 2;
    }

    buffer.resize(capacity);
}

int GeometryPool::get_free_vertex_buffer_offset() {
    if (m_objects.empty())
        return 0;

    return m_objects.back()->get_vertex_buffer_offset() + m_objects.back()->get_vertex_buffer_length();
}

int GeometryPool::allocate_buffer(int size) {
    auto free_index = get_free_vertex_buffer_offset();
    auto& storage = get_vertex_buffer()->get_storage();

    if (free_index + size >= storage.size()) {
        defragment_buffer(INT_MAX);
        free_index = get_free_vertex_buffer_offset();
        extend_buffer(storage, free_index + size);
    }

    return free_index;
}

void GeometryPool::insert_offsets_to_arrays(int free_index, int buffer_stride) {
    start_indices.push_back(free_index / SceneVertex::length);
    size_array.push_back(buffer_stride / SceneVertex::length);
}

void GeometryPool::remove_offset_from_arrays(int offset) {
    offset /= SceneVertex::length;

    auto array_it = std::lower_bound(start_indices.begin(), start_indices.end(), offset) - start_indices.begin();

    start_indices.erase(start_indices.begin() + array_it);
    size_array.erase(size_array.begin() + array_it);

    //    std::cout << "Removed array entry on index " << (array_it) << "\n";
}

void GeometryPool::adjust_offset_from_arrays(int offset, int new_offset) {
    offset /= SceneVertex::length;
    new_offset /= SceneVertex::length;

    auto array_it = std::lower_bound(start_indices.begin(), start_indices.end(), offset);

    *array_it = new_offset;
}

void GeometryPool::destroy_object(GeometryObject* object) {
    for (auto& child : object->get_children()) {
        destroy_object(child);
    }

    m_dirty_transform_objects.erase(object);

    object->get_children().clear();

    auto old_defragmented_handle = m_defragmented_elements;

    remove_offset_from_arrays(object->get_vertex_buffer_offset());

    m_matrix_buffer_index_pool.release_index(object->get_matrix_buffer_index());

    //    std::cout << "Deleted object handle on index " << object->m_vertex_buffer_offset << " with stride of " << object->m_vertex_buffer_length << " floats\n";

    if (!old_defragmented_handle || old_defragmented_handle->get_matrix_buffer_index() >= object->get_matrix_buffer_index()) {
        m_defragmented_elements = object->get_prev();
    }

    m_objects.remove(object);
    object->set_geometry_pool(nullptr);
}

std::unique_ptr<GeometryObject> GeometryPool::create_object(const GeometryObjectConfig& object_config, GeometryObject* parent) {
    auto buffer_stride = (int)object_config.m_mesh.size() * SceneVertex::length;
    auto free_index = allocate_buffer(buffer_stride);

    //    std::cout << "Adding object handle on index " << free_index << " with stride of " << buffer_stride << " floats\n";

    auto matrix_index = create_matrix();

    auto object = std::make_unique<GeometryObject>(this, free_index, buffer_stride, matrix_index, parent);
    m_objects.push_back(object.get());
    object->set_needs_transform_update();

    insert_offsets_to_arrays(free_index, buffer_stride);

    copy_geometry(free_index, object_config.m_mesh, matrix_index);

    return object;
}

void GeometryPool::copy_matrix(int index, const Matrix4f& matrix) {
    auto& storage = get_matrix_buffer()->get_storage();

    for (int i = 0; i < 16; i++) {
        storage[index * transform_stride + i] = matrix.m_data[i];
    }

    get_matrix_buffer()->mark_dirty(index * transform_stride, (index + 1) * transform_stride);
}

void GeometryPool::copy_geometry(int offset, const std::vector<SceneVertex>& vertices, int matrix_index) {
    auto buffer_stride = (int)vertices.size() * SceneVertex::length;
    auto& storage = get_vertex_buffer()->get_storage();

    int start = offset;

    for (auto& vertex : vertices) {
        vertex.write_vertex(&storage[offset], matrix_index);
        offset += SceneVertex::length;
    }

    get_vertex_buffer()->mark_dirty(start, start + buffer_stride);
}

void GeometryPool::update_transforms() {
    for (auto& object : m_dirty_transform_objects) {
        object->update_transform();
        object->update_subsidiary_transformations();
    }
    m_dirty_transform_objects.clear();
}

void GeometryPool::update_object_transform(GeometryObject* object) {
    auto index = object->get_matrix_buffer_index();
    copy_matrix(index, object->get_world_transform());
}

void GeometryPool::update_transform_delayed(GeometryObject* object) {
    m_dirty_transform_objects.insert(object);
}

std::unique_ptr<Material> GeometryPool::create_material() {
    auto index = m_material_buffer_index_pool.get_next();
    auto material = std::make_unique<Material>(this);

    material->set_buffer_index(index);
    extend_buffer(get_material_buffer()->get_storage(), (index + 1) * material_stride);
    get_material_buffer()->mark_dirty(index * material_stride, (index + 1) * material_stride);

    m_materials.insert(material.get());
    return material;
}

void GeometryPool::destroy_material(Material* material) {
    m_material_buffer_index_pool.release_index(material->get_buffer_index());
    m_dirty_materials.erase(material);
    m_materials.erase(material);
    material->set_geometry_pool(nullptr);
}

void GeometryPool::update_material_delayed(Material* material) {
    m_dirty_materials.insert(material);
}

void GeometryPool::update_materials() {
    for (auto& material : m_dirty_materials)
        update_material(material);
    m_dirty_materials.clear();
}

void GeometryPool::update_material(Material* material) {
    auto index = material->get_buffer_index();

    auto& storage = get_material_buffer()->get_storage();

    for (int i = 0; i < 3; i++) {
        storage[index * material_stride + i] = material->get_color()[i];
    }
    storage[index * material_stride + 3] = material->get_grid() ? 1 : 0;
    storage[index * material_stride + 4] = material->get_specular_factor();

    get_material_buffer()->mark_dirty(index * material_stride, (index + 1) * material_stride);
}

int GeometryPool::create_matrix() {
    int matrix_index = m_matrix_buffer_index_pool.get_next();
    extend_buffer(get_matrix_buffer()->get_storage(), (matrix_index + 1) * transform_stride);
    return matrix_index;
}

SceneStorageBuffer* GeometryPool::get_vertex_buffer() {
    return m_drawer->m_vertex_buffer.get();
}
SceneStorageBuffer* GeometryPool::get_material_buffer() {
    return m_drawer->m_material_buffer.get();
}
SceneStorageBuffer* GeometryPool::get_matrix_buffer() {
    return m_drawer->m_transform_buffer.get();
}
