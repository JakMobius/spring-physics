#pragma once

#include "../../base/gpu-window.hpp"
#include "../../base/smart-buffer/smart-buffer.hpp"
#include "../../world/geometry-pool.hpp"
#include "../camera/camera.hpp"
#include "etna/basic-attachment-manager.hpp"
#include "etna/image.hpp"
#include "etna/vk-wrappers.hpp"
#include "scene-buffer.hpp"
#include <etna/command-queue/command-queue-pool.hpp>

class RenderingContext {
public:
    RenderingContext(GpuWindow& window) : m_gpu_window(window) {
        for(int i = 0; i < 3; i++) {
            m_clear_color.set(i, pow(m_clear_color[i], 1.0f / 2.2f));
        }
    }
    GpuWindow& m_gpu_window;

    Camera* m_camera{};
    Matrix4f m_shadow_mapping_matrix{};
    Vec3f m_ambient_color = Vec3f{0.004380f, 0.036600f, 0.100000f};
    Vec3f m_sun_color = Vec3f{1.000000f, 0.806200f, 0.428500f};
    Vec3f m_clear_color{0.08535, 0.3675, 0.73755};
    Vec3f m_sun_direction{0.3f, -0.8f, 0.5f};
    Vec3f m_fog_color{0.08535, 0.3675, 0.73755};
    float m_fog_amount = 0.002;
    float m_shadow_map_size = 4096;

    VK::Fence m_flight_fence{};
    VK::Semaphore m_image_available_semaphore{};
    VK::Semaphore m_render_finished_semaphore{};

    uint32_t m_swapchain_image_index = -1;

    std::unique_ptr<Etna::CommandQueuePool> m_graphics_command_queue_pool;
    std::unique_ptr<VK::DescriptorSetArray> m_scene_descriptor_set_array{};
    std::unique_ptr<VK::DescriptorSetArray> m_particles_descriptor_set_array{};

    VK::DescriptorSetLayout m_transforms_descriptor_set_layout{};
    VK::DescriptorSetLayout m_materials_descriptor_set_layout{};
    VK::DescriptorSetLayout m_shadowing_descriptor_set_layout{};
    VK::DescriptorSetLayout m_shadow_mapping_descriptor_set_layout{};

    VK::DescriptorSetLayout m_particles_descriptor_set_layout{};

    VK::DescriptorPool m_descriptor_pool{};
    VK::CommandPool m_command_pool;

    std::unique_ptr<SceneStorageBuffer> m_vertex_buffer{};
    std::unique_ptr<SceneStorageBuffer> m_transform_buffer{};
    std::unique_ptr<SceneStorageBuffer> m_material_buffer{};
    std::unique_ptr<GeometryPool> m_geometry_pool{};

    std::unique_ptr<SceneStorageBuffer> m_particle_vertex_buffer{};

    std::unique_ptr<Etna::Image> m_color_image {};
    std::unique_ptr<Etna::Image> m_depth_image {};

    VK::CommandBuffer m_render_command_buffer{};

    Etna::Image* m_shadow_color_image = nullptr;
    VK::UnownedImageView m_shadow_map_image_view;

    VK::Sampler m_shadow_sampler{};
    VK::Sampler m_shadow_map_sampler{};

    VK::Swapchain m_swapchain{};
    VkSurfaceFormatKHR m_surface_format{};
    VkExtent2D m_swapchain_extent{};
    std::vector<VK::UnownedImage> m_swapchain_images{};

    VkSampleCountFlagBits m_msaa_samples = VK_SAMPLE_COUNT_1_BIT;
    bool m_needs_resize = false;

    VkFormat find_depth_format();

    void initialize();
    void create_descriptor_set_layouts();
    void create_vertex_buffer();
    void create_material_buffer();
    void create_transform_buffer();
    void create_particle_buffer();
    void create_descriptor_pool();
    void create_descriptor_sets();
    void create_sync_objects();
    void create_swapchain();
    void handle_swapchain_update();
    void create_shadow_sampler();
    void create_shadow_map_sampler();
    void create_depth_color_images();
    void update_buffers();

    void insert_color_barrier(VK::CommandBuffer& command_buffer);
    void insert_depth_barrier(VK::CommandBuffer& command_buffer);
};