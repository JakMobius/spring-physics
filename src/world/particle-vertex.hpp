#pragma once

#include "etna/vk-wrappers/pipeline/vk-pipeline-vertex-array-binding.hpp"

struct ParticleVertex {
    float x, y, z;
    float r, g, b, a;

    static constexpr VK::PipelineVertexAttributeInfo position_attribute{
        VK_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 0};

    static constexpr VK::PipelineVertexAttributeInfo color_attribute{
        VK_FORMAT_R32G32B32A32_SFLOAT, 1, sizeof(float) * 3};

    static constexpr int length = 7;
    static constexpr int stride = sizeof(float) * length;
};