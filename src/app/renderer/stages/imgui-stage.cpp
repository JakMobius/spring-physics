
#include "imgui-stage.hpp"
#include "etna/vk-wrappers/render-pass/vk-render-pass-begin-info.hpp"
#include "etna/vk-wrappers/render-pass/vk-render-pass-factory.hpp"
#include "etna/vk-wrappers/vk-attachment.hpp"

void ImguiStage::create_render_pass() {
    auto& window = m_ctx.m_gpu_window;

    VK::Attachment color_attachment{m_ctx.m_surface_format.format};
    color_attachment.set_samples(m_ctx.m_msaa_samples);
    color_attachment.set_initial_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    color_attachment.set_load_store_operations(VK_ATTACHMENT_LOAD_OP_LOAD,
                                               VK_ATTACHMENT_STORE_OP_STORE);
    color_attachment.set_final_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkAttachmentReference color_attachment_ref{
        0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.pResolveAttachments = nullptr;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VK::RenderPassFactory render_pass_factory;
    render_pass_factory.get_attachment_descriptions().assign(
        {color_attachment.get_description()});
    render_pass_factory.get_subpass_descriptions().assign({subpass});
    render_pass_factory.get_subpass_dependency_descriptions().assign(
        {dependency});
    m_render_pass = render_pass_factory.create(&window.get_device());
}

void ImguiStage::setup_imgui() {
    auto window = m_ctx.m_gpu_window.get_window();
    auto vk_instance = window->get_app()->get_vk_instance().get_handle();

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window->get_window(), true);
    ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vk_instance) {
        return vkGetInstanceProcAddr((VkInstance)vk_instance, function_name);
    },
                                   (void*)vk_instance);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = m_ctx.m_gpu_window.get_window()
                             ->get_app()
                             ->get_vk_instance()
                             .get_handle();
    init_info.Device = m_ctx.m_gpu_window.get_device().get_handle();
    init_info.PhysicalDevice =
        m_ctx.m_gpu_window.get_device().get_physical_device()->get_handle();
    init_info.QueueFamily = m_ctx.m_gpu_window.get_graphics_queue_family();
    init_info.Queue = m_ctx.m_gpu_window.get_device_graphics_queue().get_handle();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_imgui_descriptor_pool.get_handle();
    init_info.Allocator = nullptr;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.CheckVkResultFn = nullptr;
    // TODO: rendering interface with msaa is weird
    init_info.MSAASamples = m_ctx.m_msaa_samples;
    ImGui_ImplVulkan_Init(&init_info, m_render_pass.get_handle());

    // Upload Fonts
    auto command_buffer = m_ctx.m_command_pool.create_command_buffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    command_buffer.begin();
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer.get_handle());
    command_buffer.end();
    command_buffer.submit_and_wait(
        m_ctx.m_gpu_window.get_device_graphics_queue());
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImguiStage::handle_swapchain_update() {
    create_framebuffer();
}

void ImguiStage::deinitialize() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    m_imgui_descriptor_pool.destroy();
    ImGui::DestroyContext();
}

void ImguiStage::record_command_buffer(VK::CommandBuffer& command_buffer) {
    VkClearValue clear_values[1]{};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VK::RenderPassBeginInfo main_render_pass_begin_info(m_render_pass);
    main_render_pass_begin_info.set_framebuffer(m_framebuffer);
    main_render_pass_begin_info.get_render_area().extent = m_ctx.m_swapchain_extent;
    main_render_pass_begin_info.set_clear_values(clear_values);

    command_buffer.begin_render_pass(main_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                    command_buffer.get_handle());

    command_buffer.end_render_pass();
}

void ImguiStage::prepare_for_frame() {
}

void ImguiStage::create_imgui_descriptor_pool() {
    m_imgui_descriptor_pool =
        VK::DescriptorPoolFactory()
            .add_pool_size(VK_DESCRIPTOR_TYPE_SAMPLER, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 16)
            .add_pool_size(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 16)
            .set_flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
            .set_max_sets(16 * 11)
            .create(&m_ctx.m_gpu_window.get_device());
}

void ImguiStage::create_framebuffer() {
    VK::FramebufferFactory framebuffer_factory;
    framebuffer_factory.set_size(m_ctx.m_swapchain_extent);

    auto& attachments = framebuffer_factory.get_attachments();

    attachments.push_back(m_ctx.m_color_image->get_view());

    m_framebuffer = framebuffer_factory.create(m_render_pass);
}

void ImguiStage::initialize() {
    create_imgui_descriptor_pool();
    create_render_pass();
    setup_imgui();
    create_framebuffer();
}
