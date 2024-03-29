#pragma once

#include "../../../utils/vec3.hpp"
#include "../objects/muscle.hpp"
#include "../objects/spring-object.hpp"
#include "../objects/vertex-object.hpp"
#include <random>
#include <vector>

struct VertexConfig {
    float m_mass;
    float m_floor_friction;
    Vec3f m_position;

    void apply(VertexObject* vertex) const;
};

struct SpringConfig {
    float m_strength;
    float m_damping;
    int m_vertex_a_index;
    int m_vertex_b_index;
    float m_low_deformation_length;
    float m_high_deformation_length;

    void apply(SpringObject* spring) const;
};

struct MuscleConfig {
    int m_spring_index;
    float m_min_length_coefficient;
    float m_max_length_coefficient;

    void apply(Muscle* muscle, SpringObject* spring) const;
};

struct SurfaceConfig {
    int m_vertex_a_index;
    int m_vertex_b_index;
    int m_vertex_c_index;
};

struct JetConfig {
    int m_head_vertex_index;
    int m_tail_vertex_index;
    float m_max_force;
};

struct CreatureConfig {
    std::vector<VertexConfig> m_vertices;
    std::vector<SpringConfig> m_springs;
    std::vector<MuscleConfig> m_muscles;
    std::vector<SurfaceConfig> m_surfaces;
    std::vector<JetConfig> m_jets;
};