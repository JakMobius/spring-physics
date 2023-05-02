#pragma once

#include "../camera/controllers/camera-controller.hpp"
#include "../camera/perspective_camera.hpp"
#include "base-renderer.hpp"
#include "etna/basic-attachment-manager.hpp"
#include "etna/image.hpp"
#include "etna/vk-wrappers.hpp"
#include "rendering-context.hpp"
#include "scene-buffer.hpp"
#include "stages/imgui-stage.hpp"
#include "stages/light-map-bake-stage.hpp"
#include "stages/particles-stage.hpp"
#include "stages/resolve-present-stage.hpp"
#include "stages/scene-geometry-render-stage.hpp"
#include "stages/shadow-map-generation-stage.hpp"
#include <etna/command-queue/command-queue-pool.hpp>

class SceneRenderer : public BaseRenderer {
public:
    explicit SceneRenderer(GpuWindow& window);
    ~SceneRenderer() { wait_idle(); }

    void draw();
    void wait_idle();
    void initialize() override;

    RenderingContext* get_rendering_context() { return &m_rendering_context; }

private:
    void draw_frame();

    void prepare_for_frame() override;
    void handle_swapchain_update() override;
    void recreate_pipeline();

    void rebind_samplers();

    GpuWindow& m_gpu_window;
    RenderingContext m_rendering_context;
    std::unique_ptr<SceneGeometryRenderStage> m_scene_geometry_render_stage;
    std::unique_ptr<LightMapBakeStage> m_light_map_bake_stage;
    std::unique_ptr<ShadowMapGenerationStage> m_shadow_map_gen_stage;
    std::unique_ptr<ParticlesStage> m_particles_stage;
    std::unique_ptr<ResolvePresentStage> m_resolve_present_stage;
    std::unique_ptr<ImguiStage> m_imgui_stage;
};