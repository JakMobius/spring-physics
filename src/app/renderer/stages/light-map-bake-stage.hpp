#pragma once

#include "../rendering-context.hpp"
#include "../scene-buffer.hpp"
#include "etna/basic-attachment-manager.hpp"
#include "etna/command-queue/command-queue-pool.hpp"
#include "etna/image-factory.hpp"
#include "etna/image.hpp"
#include "etna/vk-wrappers.hpp"
#include "stage.hpp"

struct LightMapBakePushConstants {
    float camera_matrix[16];
    float shadow_camera_matrix[16];
    float camera_position[3];
    float pcf_scale;
    float viewport_size[2];
    float alignment1[2];
    float sun_direction[3];
    float alignment2[1];
    float sun_color[3];
    float alignment3[1];
};

class LightMapBakeStage : public Stage {
public:
    explicit LightMapBakeStage(RenderingContext& ctx) : m_ctx(ctx) {}

    void initialize() override;
    void handle_swapchain_update() override;
    void record_command_buffer(VK::CommandBuffer& command_buffer) override;
    void prepare_for_frame() override;

private:
    void create_depth_image();
    void create_render_pass();
    void create_graphics_pipeline();
    void make_texture_readable(VK::CommandBuffer& command_buffer) const;
    void update_push_constants();
    void create_framebuffer();

    RenderingContext& m_ctx;

    LightMapBakePushConstants m_push_constants{};
    VK::RenderPass m_render_pass{};
    std::unique_ptr<Etna::Image> m_color_image{};
    std::unique_ptr<Etna::Image> m_depth_image{};
    VK::Framebuffer m_framebuffer{};
    VK::PipelineLayout m_pipeline_layout{};
    VK::Pipeline m_pipeline{};
};