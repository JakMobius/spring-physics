
#include "base-renderer.hpp"


void BaseRenderer::record_command_buffer(VK::CommandBuffer& command_buffer) {
    for(auto stage : m_stages) {
        stage->record_command_buffer(command_buffer);
    }
}

void BaseRenderer::prepare_for_frame() {
    for(auto stage : m_stages) {
        stage->prepare_for_frame();
    }
}

void BaseRenderer::cleanup_pipeline() {
    for(auto stage : m_stages) {
        stage->cleanup_pipeline();
    }
}

void BaseRenderer::create_pipeline() {
    for(auto stage : m_stages) {
        stage->create_pipeline();
    }
}

void BaseRenderer::initialize() {
    create_pipeline();
}
