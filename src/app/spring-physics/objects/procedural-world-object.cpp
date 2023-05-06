
#include "procedural-world-object.hpp"
#include "../../../world/geometry-pool.hpp"
#include "../../../world/shape-generator.hpp"
#include "../world.hpp"

void ProceduralWorldObject::tick(float dt) {
    // Do nothing, the object is static
}

int nearest_power_of_2(int n) {
    n--;

    for (int i = 1; i <= (sizeof(n) * CHAR_BIT) / 2; i *= 2) {
        n |= n >> i;
    }

    return n + 1;
}

ProceduralWorldObject::ProceduralWorldObject(World* world)
    : WorldObject(world) {

//    m_perlin_noise = FastNoise::NewFromEncodedNodeTree("DQAFAAAAAAAAQAgAAAAAAD8AAAAAAA==");
    m_perlin_noise = FastNoise::NewFromEncodedNodeTree("EwCamZk+GwAbACMAARkAGwAZAB4AEwDNzMw9DwACAAAAAACAPxYAAAAAAAgAAM3MzD0AexSuPgAAAAA/AAAAAL8AAAAAwAAAAIA/AAAAAEABGQAZABsAEwAAAEBAFgABAAAA//8AAADNzMw9ARsAEwCkcD0/FgACAAAA//8AAACuR2E+ASMAARkAGwATAClcjz4WAAMAAAD//wAAAAAAAD8AAAAAPwBxPUpAAR4AHQAZAAUAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAACAvwAAAIA/AAAAAAA=");
};

ProceduralWorldObject::~ProceduralWorldObject() {
    m_world->remove_object(this);
}

void ProceduralWorldObject::mark_chunks_near(Vec2i xz_chunk, float max_distance) {
    for (auto it = m_dirty_chunks.begin(); it != m_dirty_chunks.end();) {
        auto& chunk = it->second;

        if ((chunk->m_from - xz_chunk).len_squared() < max_distance) {
            it->second->m_physics_required = false;
            it->second->m_min_camera_distance = -1;
            m_chunks.insert({it->first, std::move(it->second)});
            it = m_dirty_chunks.erase(it);
        } else {
            it++;
        }
    }
}

void ProceduralWorldObject::mark_creature_position(Vec3f position) {
    Vec2f xz = {position.x, position.z};

    Vec2i xz_chunk = ((Vec2i)(xz) / m_chunk_size) * m_chunk_size;
    mark_chunks_near(xz_chunk, (float)m_physics_distance * (float)m_physics_distance);

    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            Vec2i position = Vec2i{x, y} *= m_chunk_size;
            position += xz_chunk;
            if(!m_chunks.contains(position)) {
                create_chunk_at(position);
            }
        }
    }

    float max_physics_distance = (float)m_physics_distance * (float)m_physics_distance;

    for (auto it = m_chunks.begin(); it != m_chunks.end();) {
        auto& chunk = it->second;

        if ((chunk->m_center - xz).len_squared() < max_physics_distance) {
            it->second->m_physics_required = true;
        }

        it++;
    }
}

void ProceduralWorldObject::account_for_camera(Camera& camera) {
    Vec3f camera_position = camera.get_position();

    Vec2f xz = {camera_position.x, camera_position.z};
    Vec2i xz_chunk = ((Vec2i)(xz) / m_chunk_size) * m_chunk_size;

    mark_chunks_near(xz_chunk, (float)m_unload_distance * (float)m_unload_distance);

    for (auto it = m_chunks.begin(); it != m_chunks.end();) {
        auto& chunk = it->second;

        float distance = (chunk->m_center - xz).len_squared();
        if(chunk->m_min_camera_distance < 0) {
            chunk->m_min_camera_distance = distance;
        } else if (chunk->m_min_camera_distance > distance) {
            chunk->m_min_camera_distance = distance;
        }

        it++;
    }


    if (!m_chunks.contains(xz_chunk)) {
        m_chunk_allocator.add_chunk_placeholder(xz_chunk);
        m_chunk_allocator.mark_dirty(xz_chunk);
    }

    m_chunk_allocator.sort(xz_chunk);

    int i = 0;
    int limit = 1;

    float max_distance = m_render_distance * m_render_distance;

    while (m_chunk_allocator.has_nearest() && i < limit) {
        Vec2i coordinate = m_chunk_allocator.nearest();
        float distance = (float)(coordinate - xz_chunk).len_squared();
        if (distance > max_distance) {
            break;
        }
        m_chunk_allocator.pop_nearest();
        create_chunk_at(coordinate);
        i++;
    }
}

void ProceduralWorldObject::create_chunk_at(Vec2i pos) {
    m_chunk_allocator.remove_chunk_placeholder(pos);

    auto chunk = std::make_unique<ProceduralWorldChunk>(this);
    chunk->configure(pos, (pos + Vec2i{1, 1} * m_chunk_size));
    m_chunks.insert({pos, std::move(chunk)});

    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            Vec2i coordinate = Vec2i{x, y} *= m_chunk_size;
            coordinate += pos;
            if (m_chunks.find(coordinate) == m_chunks.end()) {
                m_chunk_allocator.add_chunk_placeholder(coordinate);
            }
        }
    }
}

void ProceduralWorldObject::sweep_dirty_placeholders() {
    for(auto& chunk : m_chunk_allocator.m_dirty_chunk_placeholders) {
        bool has_neighbours = false;
        for(int x = -1; x <= 1 && !has_neighbours; x++) {
            for(int y = -1; y <= 1; y++) {
                Vec2i coordinate = Vec2i{x, y} *= m_chunk_size;
                coordinate += chunk;
                if(m_chunks.contains(coordinate)) {
                    has_neighbours = true;
                    break;
                }
            }
        }
        if(!has_neighbours) {
            m_chunk_allocator.remove_chunk_placeholder(chunk);
        }
    }

    m_chunk_allocator.m_dirty_chunk_placeholders.clear();
}

void ProceduralWorldObject::update_chunk_physics() {
    for (auto& chunk : m_chunks) {
        chunk.second->update_physics();
        chunk.second->update_geometry();
    }
}

void ProceduralWorldObject::begin_far_chunk_sweep() {
    std::swap(m_chunks, m_dirty_chunks);
}

void ProceduralWorldObject::commit_far_chunk_sweep() {
    for (auto& chunk : m_dirty_chunks) {
        m_chunk_allocator.mark_neighbours_dirty(chunk.second->m_from);
    }
    m_dirty_chunks.clear();
    sweep_dirty_placeholders();
}

ProceduralWorldChunk::ProceduralWorldChunk(ProceduralWorldObject* parent)
    : m_parent(parent) {}

template<typename T> void enumerate_triangles(ProceduralWorldChunk* chunk, int grid_size, T callback) {
    auto heights = chunk->get_height_map(grid_size);
    Vec3f offset = {(float)chunk->m_from.x, 0, (float)chunk->m_from.y};
    Vec2i resolution = (chunk->m_to - chunk->m_from) / grid_size + Vec2i{1, 1};
    auto f_grid_size = (float)grid_size;

    for (int y = 1; y < resolution.y; y++) {
        for (int x = 1; x < resolution.x; x++) {
            float heights_a = heights[(y - 1) * resolution.x + (x - 1)];
            float heights_b = heights[(y - 1) * resolution.x + (x)];
            float heights_c = heights[y * resolution.x + (x - 1)];
            float heights_d = heights[y * resolution.x + (x)];

            Vec2f position = {(float)x, (float)y};
            position *= f_grid_size;

            Vec3f point_a = offset + Vec3f{position.x - f_grid_size, heights_a, position.y - f_grid_size};
            Vec3f point_b = offset + Vec3f{position.x, heights_b, position.y - f_grid_size};
            Vec3f point_c = offset + Vec3f{position.x - f_grid_size, heights_c, position.y};
            Vec3f point_d = offset + Vec3f{position.x, heights_d, position.y};

            callback(point_a, point_b, point_c, point_d);
        }
    }
}

void ProceduralWorldChunk::configure(Vec2i from, Vec2i to) {
    m_center = {(float)(from.x + to.x) / 2, (float)(from.y + to.y) / 2};
    m_from = from;
    m_to = to;
}

std::vector<float> ProceduralWorldChunk::get_height_map(int grid_size) {
    Vec2i to = m_to / grid_size;
    Vec2i from = m_from / grid_size;
    Vec2i resolution = (to - from) + Vec2i{1, 1};

    std::vector<float> heights(resolution.x * resolution.y);
    m_parent->get_generator()->GenUniformGrid2D(heights.data(), from.x, from.y, resolution.x, resolution.y, m_freq * grid_size, m_parent->get_seed());
    for(int i = 0; i < heights.size(); i++) {
        heights[i] *= 20.0f;
    }

    return heights;
}

void ProceduralWorldChunk::update_physics() {
    if (!m_physics_required) {
        if(m_terrain_chunk) {
            m_terrain_chunk = nullptr;
        }
        return;
    }
    if (m_terrain_chunk != nullptr) {
        return;
    }

    m_terrain_chunk = m_parent->get_world()->get_physics_engine()->get_terrain()->create_chunk();

    enumerate_triangles(this, 1, [this](Vec3f point_a, Vec3f point_b, Vec3f point_c, Vec3f point_d) {
         m_terrain_chunk->add_triangle(point_a, point_b, point_c);
         m_terrain_chunk->add_triangle(point_b, point_d, point_c);
    });
}

void ProceduralWorldChunk::update_geometry() {
    float distance = sqrt(m_min_camera_distance);
    distance /= m_parent->get_chunk_size();
    float log_distance = log2(distance + 1);
    log_distance = std::min(log_distance, 3.0f);

    int grid_size = 1 << (int) (log_distance);

    if(m_geometry_grid_size == grid_size) {
        return;
    }

    m_geometry_grid_size = grid_size;

    GeometryPool* geometry_pool = m_parent->get_world()->get_rendering_context()->m_geometry_pool.get();

    ShapeGenerator shape_generator;

    m_material = geometry_pool->create_material();

//    if(grid_size == 1) {
//        m_material->set_color({0.9f, 0.4f, 0.4f});
//    } else if(grid_size == 2) {
//        m_material->set_color({0.4f, 0.9f, 0.4f});
//    } else if(grid_size == 4) {
//        m_material->set_color({0.4f, 0.4f, 0.9f});
//    } else if(grid_size == 8) {
//        m_material->set_color({0.4f, 0.9f, 0.9f});
//    }

    m_material->set_color({0.4f, 0.9f, 0.4f});
    m_material->set_grid(true);

    Vec2i resolution = (m_to - m_from) / grid_size + Vec2i{1, 1};
    shape_generator.reserve_vertices(resolution.x * resolution.y * 2 * 3);

    enumerate_triangles(this, grid_size, [this, &shape_generator](Vec3f point_a, Vec3f point_b, Vec3f point_c, Vec3f point_d) {
        shape_generator.add_triangle(point_a, point_b, point_c, m_material.get());
        shape_generator.add_triangle(point_b, point_d, point_c, m_material.get());
    });

    m_geometry_object = geometry_pool->create_object({shape_generator.get_mesh()}, nullptr);
}

void ChunkAllocator::add_chunk_placeholder(Vec2i chunk) {
    if (has_chunk_placeholder(chunk)) {
        return;
    }

    m_chunk_placeholder_set.insert(chunk);
    m_chunk_placeholders.push_back(chunk);
}

void ChunkAllocator::remove_chunk_placeholder(Vec2i chunk) {
    if (!has_chunk_placeholder(chunk)) {
        return;
    }

    m_chunk_placeholder_set.erase(chunk);
    m_chunk_placeholders.erase(std::remove(m_chunk_placeholders.begin(), m_chunk_placeholders.end(), chunk), m_chunk_placeholders.end());
}

void ChunkAllocator::mark_dirty(Vec2i chunk) {
    if (m_chunk_placeholder_set.contains(chunk)) {
        m_dirty_chunk_placeholders.insert(chunk);
    }
}

void ChunkAllocator::mark_neighbours_dirty(Vec2i chunk) {
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            if (x == 0 && y == 0)
                continue;
            auto coordinate = chunk + Vec2i{x, y};
            mark_dirty(coordinate);
        }
    }
}

void ChunkAllocator::sort(Vec2i position) {
    std::sort(m_chunk_placeholders.begin(), m_chunk_placeholders.end(), [&](Vec2i a, Vec2i b) {
      return (a - position).len_squared() > (b - position).len_squared();
    });
}

void ChunkAllocator::pop_nearest() {
    Vec2i chunk = m_chunk_placeholders.back();
    m_chunk_placeholder_set.erase(chunk);
    m_chunk_placeholders.pop_back();
}
