#pragma once

#include "../../utils/vec3.hpp"
#include "../../utils/vec4.hpp"
#include "../../world/particle-vertex.hpp"
#include "../renderer/rendering-context.hpp"
#include <vector>

struct Particle {
    Vec3f m_position;
    Vec3f m_velocity;
    Vec4f m_color;
    float m_lifetime;
};

class ParticleHost {
    std::vector<Particle> m_particles;
    Vec3f m_gravity = Vec3f(0, -9.81f, 0);

public:
    void tick(float dt) {
        for (int i = 0; i < m_particles.size();) {
            Particle& particle = m_particles[i];
            if (particle.m_lifetime > 0) {
                particle.m_color.set(3, particle.m_color[3] * (1 - dt / particle.m_lifetime));
                particle.m_lifetime -= dt;
                particle.m_position += particle.m_velocity * dt;
                particle.m_velocity += m_gravity * dt;

                if (particle.m_lifetime < 0) {
                    std::swap(m_particles[i], m_particles.back());
                    m_particles.pop_back();
                    continue;
                }
            }
            i++;
        }
    }

    void add_particle(const Particle &particle) {
        if(particle.m_lifetime > 0) {
            m_particles.push_back(particle);
        }
    }

    const std::vector<Particle>& get_particles() {
        return m_particles;
    }

    void upload_particles(RenderingContext& context) {
        auto& particle_vertex_buffer = context.m_particle_vertex_buffer;
        particle_vertex_buffer->get_storage().resize(m_particles.size() * 7 * 6);
        auto *gpu_particles = (ParticleVertex*) particle_vertex_buffer->get_storage().data();

        float particle_size = 0.1f;
        auto camera = context.m_camera;

        Vec3f camera_right = camera->get_right() * particle_size;
        Vec3f camera_up = camera->get_top() * particle_size;

        for (int i = 0; i < m_particles.size(); i++) {
            const Particle *particle = m_particles.data() + i;
            ParticleVertex*gpu_particle = gpu_particles + i * 6;

            Vec3f position = particle->m_position;

            for(int j = 0; j < 6; j++) {
                memcpy(&gpu_particle[j].r, &particle->m_color, sizeof(float) * 4);
            }

            position -= camera_right;
            position -= camera_up;

            memcpy(&gpu_particle[0].x, &position, sizeof(float) * 3);
            memcpy(&gpu_particle[3].x, &position, sizeof(float) * 3);

            position += camera_right * 2;

            memcpy(&gpu_particle[1].x, &position, sizeof(float) * 3);

            position += camera_up * 2;

            memcpy(&gpu_particle[2].x, &position, sizeof(float) * 3);
            memcpy(&gpu_particle[4].x, &position, sizeof(float) * 3);

            position -= camera_right * 2;

            memcpy(&gpu_particle[5].x, &position, sizeof(float) * 3);
        }

        particle_vertex_buffer->mark_dirty(0, particle_vertex_buffer->get_storage().size());
    }
};