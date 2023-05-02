
#include "rendering-context.hpp"
#include "etna/image-factory.hpp"

void RenderingContext::create_sync_objects() {
    m_image_available_semaphore = VK::Semaphore::create(&m_gpu_window.get_device());
    m_render_finished_semaphore = VK::Semaphore::create(&m_gpu_window.get_device());
    m_flight_fence = VK::Fence::create(&m_gpu_window.get_device(), VK_FENCE_CREATE_SIGNALED_BIT);
}

void RenderingContext::create_descriptor_pool() {
    m_descriptor_pool = VK::DescriptorPoolFactory()
                                .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
                                .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
                                .set_flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                                .set_max_sets(4)
                                .create(&m_gpu_window.get_device());
}

void RenderingContext::create_descriptor_sets() {
    m_scene_descriptor_set_array = std::make_unique<VK::DescriptorSetArray>(m_descriptor_pool);
    m_scene_descriptor_set_array->get_layouts().push_back(m_transforms_descriptor_set_layout.get_handle());
    m_scene_descriptor_set_array->get_layouts().push_back(m_materials_descriptor_set_layout.get_handle());
    m_scene_descriptor_set_array->get_layouts().push_back(m_shadowing_descriptor_set_layout.get_handle());
    m_scene_descriptor_set_array->get_layouts().push_back(m_shadow_mapping_descriptor_set_layout.get_handle());
    m_scene_descriptor_set_array->create();

    //    m_particles_descriptor_set_array = std::make_unique<VK::DescriptorSetArray>(m_descriptor_pool);
    //    m_particles_descriptor_set_array->create();
}

VkFormat RenderingContext::find_depth_format() {
    return m_gpu_window.get_device().get_physical_device()->find_supported_format(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void RenderingContext::create_transform_buffer() {
    SmartBufferFactory factory{};
    factory.set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_transform_buffer = std::make_unique<SceneStorageBuffer>(&m_gpu_window.get_device(), factory);
    m_transform_buffer->bind(m_scene_descriptor_set_array.get(), 0, 0);
}

void RenderingContext::create_vertex_buffer() {
    SmartBufferFactory factory{};
    factory.set_usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_vertex_buffer = std::make_unique<SceneStorageBuffer>(&m_gpu_window.get_device(), factory);
}

void RenderingContext::create_material_buffer() {
    SmartBufferFactory factory{};
    factory.set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_material_buffer = std::make_unique<SceneStorageBuffer>(&m_gpu_window.get_device(), factory);
    m_material_buffer->bind(m_scene_descriptor_set_array.get(), 1, 0);
}

void RenderingContext::create_particle_buffer() {
    SmartBufferFactory factory{};
    factory.set_usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_particle_vertex_buffer = std::make_unique<SceneStorageBuffer>(&m_gpu_window.get_device(), factory);
}

void RenderingContext::initialize() {
    m_geometry_pool = std::make_unique<GeometryPool>(this);
    m_msaa_samples = m_gpu_window.get_device().get_physical_device()->get_max_usable_sample_count();

    m_command_pool = VK::CommandPool::create(
            &m_gpu_window.get_device(),
            m_gpu_window.get_graphics_queue_family(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    m_graphics_command_queue_pool = std::make_unique<Etna::CommandQueuePool>(m_command_pool, m_gpu_window.get_device_graphics_queue());

    VK::UnownedSurface surface = m_gpu_window.get_vk_surface();

    m_surface_format = VkSurfaceFormatKHR({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR });

    auto physical_device = m_gpu_window.get_device().get_physical_device();
    if(!physical_device->supports_surface_format(surface, m_surface_format)) {
      m_surface_format = physical_device->get_supported_surface_formats(surface)[0];
    }

    m_render_command_buffer = m_command_pool.create_command_buffer();

    create_sync_objects();
    create_descriptor_pool();
    create_descriptor_set_layouts();
    create_descriptor_sets();
    create_vertex_buffer();
    create_material_buffer();
    create_particle_buffer();
    create_transform_buffer();
    create_shadow_sampler();
    create_shadow_map_sampler();

    create_swapchain();
    create_depth_color_images();
}

void RenderingContext::update_buffers() {
    m_vertex_buffer->ensure_size();
    m_transform_buffer->ensure_size();
    m_material_buffer->ensure_size();
    m_particle_vertex_buffer->ensure_size();

    if (m_vertex_buffer->is_dirty() || m_transform_buffer->is_dirty() || m_material_buffer->is_dirty() || m_particle_vertex_buffer->is_dirty()) {
        auto command_buffer = m_command_pool.create_command_buffer();
        command_buffer.begin(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        m_vertex_buffer->update(command_buffer);
        m_transform_buffer->update(command_buffer);
        m_material_buffer->update(command_buffer);
        m_particle_vertex_buffer->update(command_buffer);

        command_buffer.end();
        command_buffer.submit_and_wait(m_gpu_window.get_device_graphics_queue());
    }
}

void RenderingContext::create_descriptor_set_layouts() {
    VK::DescriptorSetLayoutBinding transform_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    transform_binding.set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);

    VK::DescriptorSetLayoutBinding material_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    material_binding.set_stage_flags(VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);

    VK::DescriptorSetLayoutBinding shadowing_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    shadowing_binding.set_stage_flags(VK_SHADER_STAGE_FRAGMENT_BIT);

    VK::DescriptorSetLayoutBinding shadow_mapping_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    shadow_mapping_binding.set_stage_flags(VK_SHADER_STAGE_FRAGMENT_BIT);

    m_transforms_descriptor_set_layout = VK::DescriptorSetLayoutFactory()
                                                 .bind_descriptor(0, transform_binding)
                                                 .create(&m_gpu_window.get_device());

    m_materials_descriptor_set_layout = VK::DescriptorSetLayoutFactory()
                                                .bind_descriptor(0, material_binding)
                                                .create(&m_gpu_window.get_device());

    m_shadowing_descriptor_set_layout = VK::DescriptorSetLayoutFactory()
                                                .bind_descriptor(0, shadowing_binding)
                                                .create(&m_gpu_window.get_device());

    m_shadow_mapping_descriptor_set_layout = VK::DescriptorSetLayoutFactory()
                                                     .bind_descriptor(0, shadow_mapping_binding)
                                                     .create(&m_gpu_window.get_device());
}

void RenderingContext::handle_swapchain_update() {
    create_swapchain();
    create_depth_color_images();
}

void RenderingContext::create_swapchain() {
    auto& surface = m_gpu_window.get_vk_surface();
    auto& device = m_gpu_window.get_device();
    auto size = m_gpu_window.get_window()->get_framebuffer_size();

    VK::SwapchainFactory factory;

    auto physical_device = device.get_physical_device();
    auto swapchain_capabilities = device.get_physical_device()->get_surface_capabilities(surface);

    m_swapchain_extent = swapchain_capabilities.clamp_image_extent(size.x, size.y);
    uint32_t image_count = swapchain_capabilities.get_optimal_chain_image_count();

    if(physical_device->supports_surface_present_mode(surface, VK_PRESENT_MODE_MAILBOX_KHR)) {
        factory.set_present_mode(VK_PRESENT_MODE_MAILBOX_KHR);
    } else {
        factory.set_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    }

    factory.set_surface(surface);
    factory.set_min_image_count(image_count);
    factory.set_image_format(m_surface_format.format);
    factory.set_image_color_space(m_surface_format.colorSpace);
    factory.set_image_extent(m_swapchain_extent);
    factory.set_image_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    factory.set_pre_transform(swapchain_capabilities.get_current_transform());
    factory.set_composite_alpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
    factory.set_clipped(VK_TRUE);

    uint32_t graphics_queue_family = m_gpu_window.get_graphics_queue_family();
    uint32_t present_queue_family = m_gpu_window.get_present_queue_family();

    if(graphics_queue_family != present_queue_family) {
        factory.set_image_sharing_mode(VK_SHARING_MODE_CONCURRENT);
        factory.get_queue_family_indices().assign({graphics_queue_family, present_queue_family});
    } else {
        factory.set_image_sharing_mode(VK_SHARING_MODE_EXCLUSIVE);
    }

    m_swapchain = factory.create(&device);
    m_swapchain_images = m_swapchain.get_swapchain_images();
}

void RenderingContext::create_shadow_sampler() {
    VK::SamplerFactory factory{};
    factory.set_mag_filter(VK_FILTER_LINEAR);
    factory.set_min_filter(VK_FILTER_LINEAR);
    factory.set_address_mode_u(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    factory.set_address_mode_v(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    factory.set_address_mode_w(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    factory.set_border_color(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
    factory.set_compare_op(VK_COMPARE_OP_LESS);
    factory.set_compare_enable(true);
    factory.set_anisotropy_enable(false);
    factory.set_unnormalized_coordinates(false);
    factory.set_mipmap_mode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
    factory.set_mip_lod_bias(0.0f);
    factory.set_min_lod(0.0f);
    factory.set_max_lod(1.0f);

    m_shadow_sampler = factory.create(&m_gpu_window.get_device());
}

void RenderingContext::create_shadow_map_sampler() {
    VK::SamplerFactory factory{};
    factory.set_mag_filter(VK_FILTER_LINEAR);
    factory.set_min_filter(VK_FILTER_LINEAR);
    factory.set_address_mode_u(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    factory.set_address_mode_v(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    factory.set_address_mode_w(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    factory.set_border_color(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
    factory.set_compare_op(VK_COMPARE_OP_LESS);
    factory.set_compare_enable(true);
    factory.set_anisotropy_enable(false);
    factory.set_unnormalized_coordinates(false);
    factory.set_mipmap_mode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
    factory.set_mip_lod_bias(0.0f);
    factory.set_min_lod(0.0f);
    factory.set_max_lod(1.0f);

    m_shadow_map_sampler = factory.create(&m_gpu_window.get_device());
}

void RenderingContext::create_depth_color_images() {
    auto depth_format = find_depth_format();
    auto& window = m_gpu_window;

    auto color_image_factory = Etna::ImageFactory()
                                       .set_samples(m_msaa_samples)
                                       .set_extent({m_swapchain_extent.width, m_swapchain_extent.height, 1})
                                       .set_format(m_surface_format.format)
                                       .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
                                       .set_aspect_mask(VK_IMAGE_ASPECT_COLOR_BIT);

    auto depth_image_factory = Etna::ImageFactory()
                                       .set_samples(m_msaa_samples)
                                       .set_extent({m_swapchain_extent.width, m_swapchain_extent.height, 1})
                                       .set_format(depth_format)
                                       .set_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                       .set_aspect_mask(VK_IMAGE_ASPECT_DEPTH_BIT);

    m_color_image = std::make_unique<Etna::Image>(color_image_factory, &window.get_device());
    m_depth_image = std::make_unique<Etna::Image>(depth_image_factory, &window.get_device());
}

void RenderingContext::insert_color_barrier(VK::CommandBuffer& command_buffer) {
    VK::ImageMemoryBarrier color_barrier{m_color_image->get_image()};
    color_barrier.get_subresource_range().set_aspect_mask(VK_IMAGE_ASPECT_COLOR_BIT);
    color_barrier.set_layouts(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    color_barrier.set_access_masks(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    color_barrier.write(command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

void RenderingContext::insert_depth_barrier(VK::CommandBuffer& command_buffer) {
    VK::ImageMemoryBarrier depth_barrier{m_depth_image->get_image()};
    depth_barrier.get_subresource_range().set_aspect_mask(VK_IMAGE_ASPECT_DEPTH_BIT);
    depth_barrier.set_layouts(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    depth_barrier.set_access_masks(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    depth_barrier.write(command_buffer, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
}
