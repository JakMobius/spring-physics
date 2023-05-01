#pragma once

namespace VK {
class PipelineVertexArrayBindingState;
}

#include <etna/volk.hpp>
#include <vector>
#include <etna/vk-wrappers/vk-shader.hpp>
#include "vk-pipeline-input-vertex-state.hpp"

namespace VK {

struct PipelineVertexAttributeInfo {
    VkFormat format;
    uint32_t location;
    uint32_t offset;
};

class PipelineVertexArrayBindingState {

    uint32_t m_binding;
    VK::PipelineInputVertexState* m_input_vertex_state;
public:
    PipelineVertexArrayBindingState(VK::PipelineInputVertexState* input_vertex_state, uint32_t binding, uint32_t stride);

    void add_attribute(VkFormat format, uint32_t location, uint32_t offset);
    void add_attribute(const PipelineVertexAttributeInfo& attribute);
};

}