#pragma once

#include "stage.hpp"
#include "../rendering-context.hpp"
#include "../scene-buffer.hpp"
#include "etna/basic-attachment-manager.hpp"
#include "etna/command-queue/command-queue-pool.hpp"
#include "etna/image-factory.hpp"
#include "etna/image.hpp"
#include "etna/vk-wrappers.hpp"

struct SceneGeometryRenderPushConstants {
    float matrix[16];
    float ambient_color[3];
    float alignment0[1];
    float viewport_size[2];
    float alignment1[2];
    float fog_color[3];
    float fog_amount;
};

class SceneGeometryRenderStage : public Stage {
public:
    explicit SceneGeometryRenderStage(RenderingContext& ctx) : m_ctx(ctx) {}

    void initialize() override;
    void handle_swapchain_update() override;
    void record_command_buffer(VK::CommandBuffer& command_buffer) override;
    void prepare_for_frame() override;

private:
    void create_render_pass();
    void create_graphics_pipeline();
    void update_push_constants();
    void create_framebuffer();

    RenderingContext& m_ctx;

    SceneGeometryRenderPushConstants m_push_constants {};
    VK::RenderPass m_render_pass{};
    VK::PipelineLayout m_pipeline_layout{};
    VK::Pipeline m_pipeline{};
    VK::Framebuffer m_framebuffer{};
};