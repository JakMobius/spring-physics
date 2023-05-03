#pragma once

#include "../../camera/orthogonal_camera.hpp"
#include "../rendering-context.hpp"
#include "../scene-buffer.hpp"
#include "etna/basic-attachment-manager.hpp"
#include "etna/command-queue/command-queue-pool.hpp"
#include "etna/image-factory.hpp"
#include "etna/image.hpp"
#include "stage.hpp"

struct ShadowMapPushConstants {
    float matrix[16];
    uint32_t shadow_map_level;
};

class ShadowMapGenerationStage : public Stage {
  public:
    explicit ShadowMapGenerationStage(RenderingContext& ctx)
        : m_ctx(ctx) {
        m_shadow_map_size.width = (uint32_t)m_ctx.m_shadow_map_size;
        m_shadow_map_size.height = (uint32_t)m_ctx.m_shadow_map_size;
        create_depth_image();
        m_push_constants.resize(m_layers);
    }

    void initialize() override;
    void record_command_buffer(VK::CommandBuffer& command_buffer) override;
    void prepare_for_frame() override;

  private:
    void create_depth_image();
    void create_render_pass();
    void create_graphics_pipeline();
    void create_framebuffers();
    void make_depth_readable(VK::CommandBuffer& command_buffer);

    void update_push_constants();

    RenderingContext& m_ctx;

    VkExtent2D m_shadow_map_size{};
    VK::RenderPass m_render_pass{};
    VK::MemoryImage m_depth_image{};
    std::vector<VK::Framebuffer> m_framebuffers{};
    std::vector<VK::ImageView> m_image_layer_views{};
    std::vector<ShadowMapPushConstants> m_push_constants{};
    VK::ImageView m_image_view{};
    VK::PipelineLayout m_pipeline_layout{};
    VK::Pipeline m_pipeline{};
    uint32_t m_layers = 5;
};