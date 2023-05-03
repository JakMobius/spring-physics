#pragma once

#include "../../base/gpu-window.hpp"
#include "stages/stage.hpp"

class BaseRenderer {
  public:
    explicit BaseRenderer(){};
    virtual void initialize();
    virtual void deinitialize();

  protected:
    void record_command_buffer(VK::CommandBuffer& command_buffer);
    virtual void prepare_for_frame();
    virtual void handle_swapchain_update();

    std::vector<Stage*> m_stages;
};