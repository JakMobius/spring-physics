#pragma once

#include "../../base/gpu-window.hpp"
#include "stages/stage.hpp"

class BaseRenderer {
public:
    explicit BaseRenderer() {};
    void initialize();

protected:
    void record_command_buffer(VK::CommandBuffer& command_buffer);
    virtual void prepare_for_frame();
    virtual void cleanup_pipeline();
    virtual void create_pipeline();

    std::vector<Stage*> m_stages;
};