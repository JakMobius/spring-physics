#pragma once

#include "../rendering-context.hpp"
#include "stage.hpp"

class ResolvePresentStage : public Stage {
  public:
    explicit ResolvePresentStage(RenderingContext& ctx)
        : m_ctx(ctx) {}

    void handle_swapchain_update() override;
    void record_command_buffer(VK::CommandBuffer& command_buffer) override;

  private:
    RenderingContext& m_ctx;
};