
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

void BaseRenderer::handle_swapchain_update() {
    for(auto stage : m_stages) {
        stage->handle_swapchain_update();
    }
}

void BaseRenderer::initialize() {
    for(auto stage : m_stages) {
        stage->initialize();
    }
    handle_swapchain_update();
}

void BaseRenderer::deinitialize() {
    for(auto stage : m_stages) {
        stage->deinitialize();
    }
}
