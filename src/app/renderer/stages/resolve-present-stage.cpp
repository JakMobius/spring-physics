
#include "resolve-present-stage.hpp"
#include "../../../world/particle-vertex.hpp"
#include "particles-stage.hpp"

void ResolvePresentStage::handle_swapchain_update() {
}

void ResolvePresentStage::record_command_buffer(VK::CommandBuffer& command_buffer) {

    auto swapchain_image_index = m_ctx.m_swapchain_image_index;
    auto swapchain_image = m_ctx.m_swapchain_images[swapchain_image_index];
    auto swapchain_extent = m_ctx.m_swapchain_extent;
    VkExtent3D swapchain_extent_3d{swapchain_extent.width, swapchain_extent.height, 1};

    VK::ImageMemoryBarrier{m_ctx.m_color_image->get_image()}
        .set_layouts(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        .set_access_masks(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT)
        .write(command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VK::ImageMemoryBarrier{swapchain_image}
        .set_layouts(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        .set_access_masks(VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT)
        .write(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkImageResolve resolve{};
    resolve.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolve.srcSubresource.layerCount = 1;
    resolve.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolve.dstSubresource.layerCount = 1;
    resolve.extent = swapchain_extent_3d;

    vkCmdResolveImage(
        command_buffer.get_handle(),
        m_ctx.m_color_image->get_image().get_handle(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        swapchain_image.get_handle(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &resolve);

    VK::ImageMemoryBarrier(m_ctx.m_color_image->get_image())
        .set_layouts(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        .set_access_masks(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
        .write(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    VK::ImageMemoryBarrier(swapchain_image)
        .set_layouts(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        .set_access_masks(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT)
        .write(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
}
