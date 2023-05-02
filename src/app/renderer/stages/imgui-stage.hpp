#pragma once

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_impl_glfw.h"
#include "../../../imgui/imgui_impl_vulkan.h"
#include "../rendering-context.hpp"
#include "etna/vk-wrappers/command-buffer/vk-command-buffer.hpp"
#include "stage.hpp"

struct ImguiPushConstants {};

class ImguiStage : public Stage {
public:
  explicit ImguiStage(RenderingContext &ctx) : m_ctx(ctx) {}

  void initialize() override;
  void deinitialize() override;

  void handle_swapchain_update() override;
  void record_command_buffer(VK::CommandBuffer &command_buffer) override;
  void prepare_for_frame() override;

private:
  void setup_imgui();
  void create_framebuffer();
  void create_render_pass();
  void create_imgui_descriptor_pool();

  RenderingContext &m_ctx;

  ImguiPushConstants m_push_constants{};
  VK::RenderPass m_render_pass{};
  VK::PipelineLayout m_pipeline_layout{};
  VK::Pipeline m_pipeline{};
  VK::Framebuffer m_framebuffer{};
  VK::DescriptorPool m_imgui_descriptor_pool{};
};