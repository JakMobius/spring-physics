
#include "scene-renderer.hpp"
#include "etna/image-factory.hpp"

SceneRenderer::SceneRenderer(GpuWindow& window) : m_gpu_window(window),
                                                  m_rendering_context(window) {
    m_rendering_context.create_context();

    m_scene_geometry_render_stage = std::make_unique<SceneGeometryRenderStage>(m_rendering_context);
    m_light_map_bake_stage = std::make_unique<LightMapBakeStage>(m_rendering_context);
    m_shadow_map_gen_stage = std::make_unique<ShadowMapGenerationStage>(m_rendering_context);
    m_particles_stage = std::make_unique<ParticlesStage>(m_rendering_context);
    m_resolve_present_stage = std::make_unique<ResolvePresentStage>(m_rendering_context);

    m_stages = {
            m_shadow_map_gen_stage.get(),
            m_light_map_bake_stage.get(),
            m_scene_geometry_render_stage.get(),
            m_particles_stage.get(),
            m_resolve_present_stage.get(),
    };
}

void SceneRenderer::prepare_for_frame() {
    BaseRenderer::prepare_for_frame();

    m_rendering_context.m_geometry_pool->defragment_buffer(16);
    m_rendering_context.m_geometry_pool->update_transforms();
    m_rendering_context.m_geometry_pool->update_materials();
}

void SceneRenderer::draw() {
    prepare_for_frame();
    draw_frame();
}

void SceneRenderer::wait_idle() {
    m_rendering_context.m_gpu_window.get_device().wait_idle();
}

void SceneRenderer::draw_frame() {
    m_rendering_context.m_flight_fence.wait_one();

    // Only updating buffers after waiting for the fence to ensure that the
    // buffers are not in use by the GPU.
    m_rendering_context.update_buffers();

    auto swapchain = m_rendering_context.m_swapchain_manager->get_swapchain();
    auto result = swapchain.acquire_next_image(&m_rendering_context.m_swapchain_image_index, m_rendering_context.m_image_available_semaphore);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || m_rendering_context.m_needs_resize) {
        m_rendering_context.m_needs_resize = true;
        recreate_pipeline();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    m_rendering_context.m_flight_fence.reset_one();

    m_rendering_context.m_render_command_buffer.reset();
    m_rendering_context.m_render_command_buffer.begin(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    record_command_buffer(m_rendering_context.m_render_command_buffer);
    m_rendering_context.m_render_command_buffer.end();

    VkSemaphore signal_semaphores[] = {m_rendering_context.m_render_finished_semaphore.get_handle()};
    VkSemaphore wait_semaphores[] = {m_rendering_context.m_image_available_semaphore.get_handle()};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSwapchainKHR present_swapchains[] = {m_rendering_context.m_swapchain_manager->get_swapchain().get_handle()};
    uint32_t present_swapchain_images[] = {m_rendering_context.m_swapchain_image_index};

    m_rendering_context.m_render_command_buffer.submit(
            m_gpu_window.get_device_graphics_queue(),
            m_rendering_context.m_flight_fence,
            signal_semaphores, wait_semaphores, wait_stages);

    VK::QueuePresentInfo queue_present_info{};
    queue_present_info.set_wait_semaphores(signal_semaphores);
    queue_present_info.set_swapchains(present_swapchains);
    queue_present_info.set_images(present_swapchain_images);

    result = m_gpu_window.get_device_present_queue().present(queue_present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        m_rendering_context.m_needs_resize = true;
        recreate_pipeline();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void SceneRenderer::cleanup_pipeline() {
    BaseRenderer::cleanup_pipeline();
    m_rendering_context.cleanup_pipeline();
}

void SceneRenderer::create_pipeline() {
    m_rendering_context.create_pipeline();
    BaseRenderer::create_pipeline();

    VkDescriptorImageInfo shadow_image_info{};
    shadow_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    shadow_image_info.imageView = m_rendering_context.m_shadow_color_image->get_view().get_handle();
    shadow_image_info.sampler = m_rendering_context.m_shadow_sampler.get_handle();

    VkDescriptorImageInfo shadow_map_image_info{};
    shadow_map_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    shadow_map_image_info.imageView = m_rendering_context.m_shadow_map_image_view.get_handle();
    shadow_map_image_info.sampler = m_rendering_context.m_shadow_map_sampler.get_handle();

    VkWriteDescriptorSet shadow_image_info_write{};
    shadow_image_info_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    shadow_image_info_write.dstSet = m_rendering_context.m_scene_descriptor_set_array->get_descriptor_sets()[2];
    shadow_image_info_write.dstBinding = 0;
    shadow_image_info_write.dstArrayElement = 0;
    shadow_image_info_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadow_image_info_write.descriptorCount = 1;
    shadow_image_info_write.pImageInfo = &shadow_image_info;

    VkWriteDescriptorSet shadow_map_image_info_write{};
    shadow_map_image_info_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    shadow_map_image_info_write.dstSet = m_rendering_context.m_scene_descriptor_set_array->get_descriptor_sets()[3];
    shadow_map_image_info_write.dstBinding = 0;
    shadow_map_image_info_write.dstArrayElement = 0;
    shadow_map_image_info_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadow_map_image_info_write.descriptorCount = 1;
    shadow_map_image_info_write.pImageInfo = &shadow_map_image_info;

    std::array<VkWriteDescriptorSet, 2> write_descriptor_sets = {shadow_image_info_write, shadow_map_image_info_write};

    vkUpdateDescriptorSets(m_gpu_window.get_device().get_handle(), 2, write_descriptor_sets.data(), 0, nullptr);
}

void SceneRenderer::recreate_pipeline() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_gpu_window.get_window()->get_window(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_gpu_window.get_window()->get_window(), &width, &height);
        glfwWaitEvents();
    }

    m_gpu_window.get_device().wait_idle();

    cleanup_pipeline();
    create_pipeline();
    m_rendering_context.m_needs_resize = false;
}
