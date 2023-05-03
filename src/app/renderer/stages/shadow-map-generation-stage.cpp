
#include "shadow-map-generation-stage.hpp"

void ShadowMapGenerationStage::create_depth_image() {
    auto depth_format = m_ctx.find_depth_format();
    auto& window = m_ctx.m_gpu_window;

    VkExtent2D extent = {m_shadow_map_size.width, m_shadow_map_size.height};
    VkExtent3D extent3D{extent.width, extent.height, 1};

    auto depth_image_factory = VK::ImageFactory()
                                       .set_array_layers(m_layers)
                                       .set_image_type(VK_IMAGE_TYPE_2D)
                                       .set_samples(VK_SAMPLE_COUNT_1_BIT)
                                       .set_extent(extent3D)
                                       .set_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
                                       .set_format(depth_format);
    m_depth_image = depth_image_factory.create(&window.get_device());

    auto depth_image_view_factory = VK::ImageViewFactory()
                                            .set_format(depth_format)
                                            .set_view_type(VK_IMAGE_VIEW_TYPE_2D);

    depth_image_view_factory.get_subresource_range()
            .set_aspect_mask(VK_IMAGE_ASPECT_DEPTH_BIT);

    for (uint32_t i = 0; i < m_layers; i++) {
        depth_image_view_factory.get_subresource_range().set_base_array_layer(i);

        m_image_layer_views.push_back(depth_image_view_factory.create(&window.get_device(), m_depth_image.get_image()));
    }

    depth_image_view_factory.set_view_type(VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    depth_image_view_factory.get_subresource_range()
            .set_base_array_layer(0)
            .set_array_layers(m_layers);

    m_image_view = depth_image_view_factory.create(&window.get_device(), m_depth_image.get_image());

    m_ctx.m_shadow_map_image_view = m_image_view;
}

void ShadowMapGenerationStage::create_render_pass() {
    auto depth_format = m_ctx.find_depth_format();
    auto& window = m_ctx.m_gpu_window;

    VK::Attachment depth_attachment{depth_format};
    depth_attachment.set_samples(VK_SAMPLE_COUNT_1_BIT);
    depth_attachment.set_load_store_operations(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
    depth_attachment.set_final_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkAttachmentReference depth_attachment_ref{0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pColorAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
    subpass.pResolveAttachments = nullptr;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VK::RenderPassFactory render_pass_factory;
    render_pass_factory.get_attachment_descriptions().assign({depth_attachment.get_description()});
    render_pass_factory.get_subpass_descriptions().assign({subpass});
    render_pass_factory.get_subpass_dependency_descriptions().assign({dependency});
    m_render_pass = render_pass_factory.create(&window.get_device());
}

void ShadowMapGenerationStage::create_graphics_pipeline() {
    auto& window = m_ctx.m_gpu_window;

    VK::PipelineFactory pipeline_factory{};

    auto device = &window.get_device();

    auto vertex_shader = VK::ShaderModule::from_file(device, "resources/shaders/shadow-map-generation/vert.spv");
    auto fragment_shader = VK::ShaderModule::from_file(device, "resources/shaders/shadow-map-generation/frag.spv");

    pipeline_factory.m_shader_stages.add_shader(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
    pipeline_factory.m_shader_stages.add_shader(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    auto vertex_array_binding = pipeline_factory.m_input_vertex_state.create_binding(0, SceneVertex::stride);
    vertex_array_binding.add_attribute(SceneVertex::position_attribute);
    vertex_array_binding.add_attribute(SceneVertex::matrix_index_attribute);

    pipeline_factory.m_viewport_state.add_viewport(VK::Viewport(m_shadow_map_size));
    pipeline_factory.m_viewport_state.add_scissor(VkRect2D{{0, 0}, m_shadow_map_size});

    pipeline_factory.m_rasterization_state.set_cull_mode(VK_CULL_MODE_BACK_BIT);
    pipeline_factory.m_rasterization_state.set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pipeline_factory.m_multisampling_state.set_rasterization_samples(VK_SAMPLE_COUNT_1_BIT);

    VK::PipelineColorAttachmentState pipeline_color_attachment_states{};

    pipeline_factory.m_color_blend_state_create_info.add_color_attachment(pipeline_color_attachment_states);

    pipeline_factory.m_depth_stencil_states.set_depth_test_enable(true);
    pipeline_factory.m_depth_stencil_states.set_depth_write_enable(true);
    pipeline_factory.m_depth_stencil_states.set_depth_compare_op(VK_COMPARE_OP_LESS);

    VkPushConstantRange push_constant;
    push_constant.offset = 0;
    push_constant.size = sizeof(ShadowMapPushConstants);
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayout descriptors[]{
            m_ctx.m_transforms_descriptor_set_layout.get_handle(),
            m_ctx.m_materials_descriptor_set_layout.get_handle(),
    };
    VkPushConstantRange push_constants[]{push_constant};

    m_pipeline_layout = VK::PipelineLayout::create(&window.get_device(), descriptors, push_constants);
    m_pipeline = pipeline_factory.create(m_pipeline_layout, m_render_pass);
}

void ShadowMapGenerationStage::record_command_buffer(VK::CommandBuffer& command_buffer) {
    VkClearValue clear_values[2]{};
    clear_values[0].depthStencil = {1.0f, 0};

    auto vertex_buffer = m_ctx.m_vertex_buffer.get();

    VkDescriptorSet descriptors[]{
            m_ctx.m_scene_descriptor_set_array->get_descriptor_sets()[0],// transforms
            m_ctx.m_scene_descriptor_set_array->get_descriptor_sets()[1] // materials
    };
    VkBuffer vertex_buffers[] = {vertex_buffer->get_buffer()->get_buffer().get_handle()};
    VkDeviceSize offsets[] = {0};

    for (int i = 0; i < m_layers; i++) {
        VK::RenderPassBeginInfo shading_render_pass_begin_info(m_render_pass);
        shading_render_pass_begin_info.set_framebuffer(m_framebuffers[i]);
        shading_render_pass_begin_info.get_render_area().extent = m_shadow_map_size;
        shading_render_pass_begin_info.set_clear_values(clear_values);

        command_buffer.begin_render_pass(shading_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        command_buffer.bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        vkCmdPushConstants(command_buffer.get_handle(), m_pipeline_layout.get_handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowMapPushConstants), &m_push_constants[i]);

        command_buffer.bind_vertex_buffers(vertex_buffers, offsets);
        command_buffer.bind_descriptor_sets(VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout.get_handle(), descriptors, {});

        auto& sizes = m_ctx.m_geometry_pool->get_size_array();
        auto& indices = m_ctx.m_geometry_pool->get_start_indices();

        for(int i = 0; i < sizes.size(); i++) {
          command_buffer.draw(sizes[i], 1, indices[i], 0);
        }

        command_buffer.end_render_pass();
    }

    make_depth_readable(command_buffer);
}

void ShadowMapGenerationStage::make_depth_readable(VK::CommandBuffer& command_buffer) {
    VK::ImageMemoryBarrier barrier{m_depth_image.get_image()};
    barrier.get_subresource_range().set_aspect_mask(VK_IMAGE_ASPECT_DEPTH_BIT);
    barrier.get_subresource_range().set_array_layers(m_layers);
    barrier.get_subresource_range().set_base_array_layer(0);
    barrier.set_layouts(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    barrier.set_access_masks(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
    barrier.write(command_buffer, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void ShadowMapGenerationStage::prepare_for_frame() {
    update_push_constants();
}

void ShadowMapGenerationStage::update_push_constants() {
    for (int i = 0; i < m_layers; i++) {
        memcpy(&m_push_constants[i].matrix, &m_ctx.m_shadow_mapping_matrix.m_data, sizeof(m_ctx.m_shadow_mapping_matrix.m_data));

        m_push_constants[i].shadow_map_level = i;
    }
}

void ShadowMapGenerationStage::initialize() {
    create_render_pass();
    create_graphics_pipeline();
    create_framebuffers();
}

void ShadowMapGenerationStage::create_framebuffers() {
    VK::FramebufferFactory framebuffer_factory;
    framebuffer_factory.set_size(m_shadow_map_size);

    for (int i = 0; i < m_layers; i++) {
        framebuffer_factory.get_attachments() = {m_image_layer_views[i]};
        m_framebuffers.push_back(framebuffer_factory.create(m_render_pass));
    }
}
