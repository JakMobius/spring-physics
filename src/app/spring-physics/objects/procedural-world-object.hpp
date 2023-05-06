#pragma once

#include "../../../utils/vec2.hpp"
#include "../../../world/geometry-object.hpp"
#include "../../../world/material.hpp"
#include "../../camera/camera.hpp"
#include "../physics/terrain-polygon.hpp"
#include "FastNoise/FastNoise.h"
#include "world-object.hpp"
#include <unordered_set>
#include <vector>

class ProceduralWorldObject;

struct ProceduralWorldChunk {
    ProceduralWorldObject* m_parent = nullptr;
    std::unique_ptr<TerrainChunk> m_terrain_chunk = nullptr;
    std::unique_ptr<GeometryObject> m_geometry_object = nullptr;
    std::unique_ptr<Material> m_material = nullptr;

    Vec2f m_center{};
    Vec2i m_from{};
    Vec2i m_to{};
    float m_freq = 0.05f;
    float m_min_camera_distance = -1;
    int m_geometry_grid_size = -1;
    bool m_physics_required = false;

    ProceduralWorldChunk(ProceduralWorldObject* parent);

    void configure(Vec2i from, Vec2i to);
    void update_physics();
    void update_geometry();
    std::vector<float> get_height_map(int grid_size);

    GeometryObject* get_geometry_object() {
        return m_geometry_object.get();
    }
};

struct ChunkAllocator {
    std::vector<Vec2i> m_chunk_placeholders;
    std::unordered_set<Vec2i> m_chunk_placeholder_set;
    std::unordered_set<Vec2i> m_dirty_chunk_placeholders;

    void add_chunk_placeholder(Vec2i chunk);
    void remove_chunk_placeholder(Vec2i chunk);

    bool has_chunk_placeholder(Vec2i chunk) {
        return m_chunk_placeholder_set.find(chunk) != m_chunk_placeholder_set.end();
    }

    void mark_dirty(Vec2i chunk);
    void mark_neighbours_dirty(Vec2i chunk);

    void sort(Vec2i position);

    bool has_nearest() const {
        return !m_chunk_placeholders.empty();
    }

    Vec2i nearest() {
        return m_chunk_placeholders.back();
    }

    void pop_nearest();
};

class ProceduralWorldObject : public WorldObject {
  public:
    explicit ProceduralWorldObject(World* world);

    ~ProceduralWorldObject() override;

    void tick(float dt) override;

    FastNoise::SmartNode<>& get_generator() {
        return m_perlin_noise;
    }

    int get_chunk_size() {
        return m_chunk_size;
    }

    int get_seed() {
        return m_seed;
    }

    const std::unordered_map<Vec2i, std::unique_ptr<ProceduralWorldChunk>>& get_chunks() {
        return m_chunks;
    }

    void begin_far_chunk_sweep();
    void mark_creature_position(Vec3f position);
    void account_for_camera(Camera& camera);
    void commit_far_chunk_sweep();
    void update_chunk_physics();

  private:
    void create_chunk_at(Vec2i pos);
    void mark_chunks_near(Vec2i xz_chunk, float max_distance);
    void sweep_dirty_placeholders();

    std::unordered_map<Vec2i, std::unique_ptr<ProceduralWorldChunk>> m_chunks{};
    std::unordered_map<Vec2i, std::unique_ptr<ProceduralWorldChunk>> m_dirty_chunks{};
    FastNoise::SmartNode<> m_perlin_noise;
    int m_seed = 1234;
    int m_chunk_size = 32;
    int m_render_distance = 14 * 32;
    int m_unload_distance = 16 * 32;
    int m_physics_distance = 2 * m_chunk_size;

    ChunkAllocator m_chunk_allocator;
};