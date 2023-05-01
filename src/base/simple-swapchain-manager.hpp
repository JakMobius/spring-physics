#pragma once

#include <etna/vk-wrappers/swapchain.hpp>
#include <set>

struct SimpleSwapchainImage {
    VK::UnownedImage m_image;
    VK::ImageView m_image_view {};
    VK::Framebuffer m_framebuffer {};

    explicit SimpleSwapchainImage(const VK::UnownedImage& image): m_image(image) {}
    SimpleSwapchainImage(SimpleSwapchainImage&& move) = default;
    SimpleSwapchainImage& operator=(SimpleSwapchainImage&& move_assign) = default;
};

template<typename ImageType = SimpleSwapchainImage>
class SimpleSwapchainManager {
    VK::Device* m_device;
    VK::UnownedSurface m_surface;
    VK::Swapchain m_swapchain;
    std::vector<ImageType> m_swapchain_images {};
    std::set<uint32_t> m_family_indices {};
    VkExtent2D m_swapchain_extent {};
    VkSurfaceFormatKHR m_swapchain_format {};

public:
    SimpleSwapchainManager(const VK::UnownedSurface& surface, VK::Device* device): m_surface(surface), m_device(device) {

    }

    ~SimpleSwapchainManager() {
        destroy();
    }

    void configure_swapchain(VK::SwapchainFactory& factory, int width, int height) {
        auto physical_device = m_device->get_physical_device();

        auto swapchain_capabilities = m_device->get_physical_device()->get_surface_capabilities(m_surface);

        m_swapchain_extent = swapchain_capabilities.clamp_image_extent(width, height);
        uint32_t image_count = swapchain_capabilities.get_optimal_chain_image_count();

        m_swapchain_format = VkSurfaceFormatKHR({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR });

        // TODO: not quite optimal
        if(!physical_device->supports_surface_format(m_surface, m_swapchain_format)) {
            m_swapchain_format = physical_device->get_supported_surface_formats(m_surface)[0];
        }

        if(physical_device->supports_surface_present_mode(m_surface, VK_PRESENT_MODE_MAILBOX_KHR)) {
            factory.set_present_mode(VK_PRESENT_MODE_MAILBOX_KHR);
        } else {
            factory.set_present_mode(VK_PRESENT_MODE_FIFO_KHR);
        }

        factory.set_surface(m_surface);
        factory.set_min_image_count(image_count);
        factory.set_image_format(m_swapchain_format.format);
        factory.set_image_color_space(m_swapchain_format.colorSpace);
        factory.set_image_extent(m_swapchain_extent);
        factory.set_image_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        factory.set_pre_transform(swapchain_capabilities.get_current_transform());
        factory.set_composite_alpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
        factory.set_clipped(VK_TRUE);

        if (m_family_indices.size() > 1) {
            factory.set_image_sharing_mode(VK_SHARING_MODE_CONCURRENT);
            factory.get_queue_family_indices().assign(m_family_indices.begin(), m_family_indices.end());
        } else {
            factory.set_image_sharing_mode(VK_SHARING_MODE_EXCLUSIVE);
        }
    }

    void create_swapchain(int width, int height) {
        VK::SwapchainFactory factory;

        configure_swapchain(factory, width, height);

        m_swapchain_format.format = factory.get_image_format();
        m_swapchain_format.colorSpace = factory.get_image_color_space();
        m_swapchain_extent = factory.get_image_extent();

        m_swapchain = factory.create(m_device);

        for(auto& image : m_swapchain.get_swapchain_images()) {
            m_swapchain_images.emplace_back(image);
        }
    }

    void destroy() {
        m_swapchain.destroy();
        m_swapchain_images.clear();
    }

    VK::Device* get_device()                               { return m_device; }
    VK::UnownedSurface get_surface()                 const { return m_surface; }
    VK::UnownedSwapchain get_swapchain()             const { return m_swapchain; }

    std::vector<ImageType>& get_swapchain_images()             { return m_swapchain_images; }
    const std::vector<ImageType>& get_swapchain_images() const { return m_swapchain_images; }

    std::set<uint32_t>& get_family_indices()               { return m_family_indices; }
    const std::set<uint32_t>& get_family_indices()   const { return m_family_indices; }

    VkExtent2D get_swapchain_extent()                const { return m_swapchain_extent; }
    const VkSurfaceFormatKHR& get_swapchain_format() const { return m_swapchain_format; }
};