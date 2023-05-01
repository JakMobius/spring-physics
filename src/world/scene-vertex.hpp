#pragma once

#include "../utils/vec2.hpp"
#include "../utils/vec3.hpp"
#include "etna/vk-wrappers/pipeline/vk-pipeline-vertex-array-binding.hpp"
#include "material.hpp"

struct SceneVertex {
    Vec3f m_position;
    Vec3f m_normal;
    Vec2f m_uv;
    Material* m_material;

    SceneVertex(const Vec3f &position, const Vec3f &normal, const Vec2f& uv, Material* material) :
            m_position(position), m_normal(normal), m_uv(uv), m_material(material) {
    }

    void write_vertex(float* buffer, int matrix_index) const {
        buffer[0] = m_position.x;
        buffer[1] = m_position.y;
        buffer[2] = m_position.z;

        buffer[3] = m_normal.x;
        buffer[4] = m_normal.y;
        buffer[5] = m_normal.z;

        buffer[6] = m_uv.x;
        buffer[7] = m_uv.y;

        *((int*) &buffer[8]) = matrix_index;
        *((int*) &buffer[9]) = m_material->get_buffer_index();
    }

    static constexpr VK::PipelineVertexAttributeInfo position_attribute {
        VK_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 0
    };

    static constexpr VK::PipelineVertexAttributeInfo normal_attribute {
        VK_FORMAT_R32G32B32_SFLOAT, 1, sizeof(float) * 3
    };

    static constexpr VK::PipelineVertexAttributeInfo uv_attribute {
        VK_FORMAT_R32G32_SFLOAT, 2, sizeof(float) * 6
    };

    static constexpr VK::PipelineVertexAttributeInfo matrix_index_attribute {
        VK_FORMAT_R32_UINT, 3, sizeof(float) * 8
    };

    static constexpr VK::PipelineVertexAttributeInfo material_index_attribute {
        VK_FORMAT_R32_UINT, 4, sizeof(float) * 9
    };

    // 8 floats + two additional 32-bit indices
    static constexpr int length = 10;

    static constexpr int stride = sizeof(float) * length;
};