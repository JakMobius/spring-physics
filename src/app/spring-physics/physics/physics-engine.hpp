#pragma once

#include "../terrain/terrain.hpp"
#include "physics-thread.hpp"
#include "spring.hpp"
#include "terrain-polygon.hpp"
#include "vertex.hpp"
#include <unordered_set>
#include <vector>

class ConcurrentPhysicsEngine {
    std::vector<PhysicsThread*> m_threads{};

    //    Vec3f m_gravity{0, 0, 0};
    Vec3f m_gravity{0, -9.81, 0};
    float m_floor_level = -10.0f;
    float m_dt = 1.0f / 60.0f;
    int m_substeps_count = 40;
    int m_thread_count = 8;
    bool m_clear_forces = false;
    Terrain m_terrain{};

    template <typename Comparator, typename MemberFunc>
    PhysicsThread* choose_thread(Comparator comp, MemberFunc func);

    std::vector<TerrainPolygon*> m_terrain_polygons{};

  public:
    ConcurrentPhysicsEngine();

    ~ConcurrentPhysicsEngine();

    const std::vector<PhysicsThread*>& get_threads() {
        return m_threads;
    }

    void add_creature(Creature* creature);

    void remove_creature(Creature* creature);

    Terrain* get_terrain() {
        return &m_terrain;
    }

    void tick();

    float get_dt();

    void set_dt(float dt) {
        m_dt = dt;
    }

    int get_substeps();

    void set_substeps(int substeps) {
        m_substeps_count = substeps;
    }

    Vec3f get_gravity();

    void set_gravity(Vec3f gravity) {
        m_gravity = gravity;
    }

    float get_floor_level();

    bool threads_should_clear_forces() {
        return m_clear_forces;
    }
};