
#include "particles-stage.hpp"
#include "../../../world/particle-vertex.hpp"

void ParticlesStage::cleanup_pipeline() {
    m_pipeline.destroy();
    m_pipeline_layout.destroy();
    m_render_pass.destroy();
}

void ParticlesStage::create_render_pass() {
    auto swapchain_manager = m_ctx.m_swapchain_manager.get();
    auto depth_format = m_ctx.find_depth_format();
    auto& window = m_ctx.m_gpu_window;

    VK::Attachment color_attachment{swapchain_manager->get_swapchain_format().format};
    color_attachment.set_samples(m_ctx.m_msaa_samples);
    color_attachment.set_initial_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    color_attachment.set_load_store_operations(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
    color_attachment.set_final_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VK::Attachment depth_attachment{depth_format};
    depth_attachment.set_samples(m_ctx.m_msaa_samples);
    depth_attachment.set_initial_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    depth_attachment.set_load_store_operations(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE);
    depth_attachment.set_final_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkAttachmentReference color_attachment_ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_attachment_ref{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
    subpass.pResolveAttachments = nullptr;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VK::RenderPassFactory render_pass_factory;
    render_pass_factory.get_attachment_descriptions().assign({color_attachment.get_description(),
                                                              depth_attachment.get_description()});
    render_pass_factory.get_subpass_descriptions().assign({subpass});
    render_pass_factory.get_subpass_dependency_descriptions().assign({dependency});
    m_render_pass = render_pass_factory.create(&window.get_device());

    VK::FramebufferFactory framebuffer_factory;
    framebuffer_factory.set_size(swapchain_manager->get_swapchain_extent());

    auto& attachments = framebuffer_factory.get_attachments();

    attachments.push_back(m_ctx.m_color_image->get_view());
    attachments.push_back(m_ctx.m_depth_image->get_view());

    m_framebuffer = framebuffer_factory.create(m_render_pass);
}

void ParticlesStage::create_graphics_pipeline() {
    auto& window = m_ctx.m_gpu_window;
    auto device = &window.get_device();
    auto swapchain_manager = m_ctx.m_swapchain_manager.get();

    auto vertex_shader = VK::ShaderModule::from_file(device, "resources/shaders/particles/vert.spv");
    auto fragment_shader = VK::ShaderModule::from_file(device, "resources/shaders/particles/frag.spv");

    VK::PipelineFactory pipeline_factory{};
    pipeline_factory.m_shader_stages.add_shader(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
    pipeline_factory.m_shader_stages.add_shader(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    auto vertex_array_binding = pipeline_factory.m_input_vertex_state.create_binding(0, ParticleVertex::stride);
    vertex_array_binding.add_attribute(ParticleVertex::position_attribute);
    vertex_array_binding.add_attribute(ParticleVertex::color_attribute);

    pipeline_factory.m_viewport_state.add_viewport(VK::Viewport(swapchain_manager->get_swapchain_extent()));
    pipeline_factory.m_viewport_state.add_scissor(VkRect2D{{0, 0}, swapchain_manager->get_swapchain_extent()});

    pipeline_factory.m_rasterization_state.set_cull_mode(VK_CULL_MODE_BACK_BIT);
    pipeline_factory.m_rasterization_state.set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pipeline_factory.m_multisampling_state.set_rasterization_samples(m_ctx.m_msaa_samples);

    VK::PipelineColorAttachmentState pipeline_color_attachment_states{};
    pipeline_color_attachment_states.set_color_write_mask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    pipeline_color_attachment_states.set_blend_enable(true);
    pipeline_color_attachment_states.set_src_color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA);
    pipeline_color_attachment_states.set_dst_color_blend_factor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
    pipeline_color_attachment_states.set_color_blend_op(VK_BLEND_OP_ADD);
    pipeline_color_attachment_states.set_src_alpha_blend_factor(VK_BLEND_FACTOR_ONE);
    pipeline_color_attachment_states.set_dst_alpha_blend_factor(VK_BLEND_FACTOR_ZERO);
    pipeline_color_attachment_states.set_alpha_blend_op(VK_BLEND_OP_ADD);

    pipeline_factory.m_color_blend_state_create_info.add_color_attachment(pipeline_color_attachment_states);
    pipeline_factory.m_color_blend_state_create_info.set_logic_op_enable(false);
    pipeline_factory.m_color_blend_state_create_info.set_logic_op(VK_LOGIC_OP_COPY);

    pipeline_factory.m_depth_stencil_states.set_depth_test_enable(true);
    pipeline_factory.m_depth_stencil_states.set_depth_write_enable(true);
    pipeline_factory.m_depth_stencil_states.set_depth_compare_op(VK_COMPARE_OP_LESS);

    VkPushConstantRange push_constant;
    push_constant.offset = 0;
    push_constant.size = sizeof(ParticlesPushConstants);
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPushConstantRange push_constants[]{push_constant};

    m_pipeline_layout = VK::PipelineLayout::create(&window.get_device(), {}, push_constants);
    m_pipeline = pipeline_factory.create(m_pipeline_layout, m_render_pass);
}

void ParticlesStage::create_pipeline() {
    create_render_pass();
    create_graphics_pipeline();
}

void ParticlesStage::record_command_buffer(VK::CommandBuffer& command_buffer) {

    VkClearValue clear_values[3]{};
    clear_values[0].color = {{m_ctx.m_clear_color[0], m_ctx.m_clear_color[1], m_ctx.m_clear_color[2], 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    auto vertex_buffer = m_ctx.m_particle_vertex_buffer.get();

    if (vertex_buffer->get_storage().empty()) {
        return;
    }

    VkBuffer vertex_buffers[] = {vertex_buffer->get_buffer()->get_buffer().get_handle()};
    VkDeviceSize offsets[] = {0};

    auto swapchain_manager = m_ctx.m_swapchain_manager.get();

    VK::RenderPassBeginInfo main_render_pass_begin_info(m_render_pass);
    main_render_pass_begin_info.set_framebuffer(m_framebuffer);
    main_render_pass_begin_info.get_render_area().extent = swapchain_manager->get_swapchain_extent();
    main_render_pass_begin_info.set_clear_values(clear_values);

    command_buffer.begin_render_pass(main_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    command_buffer.bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    vkCmdPushConstants(command_buffer.get_handle(), m_pipeline_layout.get_handle(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ParticlesPushConstants), &m_push_constants);

    command_buffer.bind_vertex_buffers(vertex_buffers, offsets);
    command_buffer.draw(vertex_buffer->get_storage().size() / ParticleVertex::length, 1, 0, 0);
    command_buffer.end_render_pass();
}

void ParticlesStage::update_push_constants() {
    auto& camera_matrix = m_ctx.m_camera->get_matrix().m_data;

    memcpy(&m_push_constants.matrix, &camera_matrix, sizeof(camera_matrix));

    m_push_constants.fog_color[0] = m_ctx.m_fog_color[0];
    m_push_constants.fog_color[1] = m_ctx.m_fog_color[1];
    m_push_constants.fog_color[2] = m_ctx.m_fog_color[2];

    m_push_constants.fog_amount = m_ctx.m_fog_amount;
}

void ParticlesStage::prepare_for_frame() {
    update_push_constants();
}
