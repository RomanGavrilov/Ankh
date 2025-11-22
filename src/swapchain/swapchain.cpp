#include "swapchain/swapchain.hpp"
#include "core/physical-device.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace ankh
{

    static SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device,
                                                           VkSurfaceKHR surface)
    {
        SwapChainSupportDetails details{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    static VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR> &formats)
    {
        for (const auto &f : formats)
        {
            if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
                f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return f;
        }
        return formats[0];
    }

    static VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR> &modes)
    {
        for (auto m : modes)
            if (m == VK_PRESENT_MODE_MAILBOX_KHR)
                return m;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    static VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR &caps, GLFWwindow *window)
    {
        if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return caps.currentExtent;

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        VkExtent2D actual{
            static_cast<uint32_t>(w),
            static_cast<uint32_t>(h)};

        actual.width = std::clamp(actual.width,
                                  caps.minImageExtent.width,
                                  caps.maxImageExtent.width);
        actual.height = std::clamp(actual.height,
                                   caps.minImageExtent.height,
                                   caps.maxImageExtent.height);
        return actual;
    }

    Swapchain::Swapchain(const PhysicalDevice &phys,
                         VkDevice device,
                         VkSurfaceKHR surface,
                         GLFWwindow *window)
        : m_device(device), m_surface(surface)
    {
        create(phys, window);
    }

    Swapchain::~Swapchain()
    {
        cleanup();
    }

    void Swapchain::cleanup()
    {
        for (auto view : m_image_views)
            vkDestroyImageView(m_device, view, nullptr);
        m_image_views.clear();

        if (m_swapchain)
        {
            vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
    }

    void Swapchain::create(const PhysicalDevice &phys, GLFWwindow *window)
    {
        auto support = query_swapchain_support(phys.handle(), m_surface);

        VkSurfaceFormatKHR surfaceFormat = choose_surface_format(support.formats);
        VkPresentModeKHR presentMode = choose_present_mode(support.presentModes);
        VkExtent2D extent = choose_extent(support.capabilities, window);

        uint32_t imageCount = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0 &&
            imageCount > support.capabilities.maxImageCount)
        {
            imageCount = support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR ci{};
        ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        ci.surface = m_surface;
        ci.minImageCount = imageCount;
        ci.imageFormat = surfaceFormat.format;
        ci.imageColorSpace = surfaceFormat.colorSpace;
        ci.imageExtent = extent;
        ci.imageArrayLayers = 1;
        ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = phys.queues();
        uint32_t queueFamilyIndices[] = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            ci.queueFamilyIndexCount = 2;
            ci.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        ci.preTransform = support.capabilities.currentTransform;
        ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        ci.presentMode = presentMode;
        ci.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(m_device, &ci, nullptr, &m_swapchain) != VK_SUCCESS)
            throw std::runtime_error("failed to create swapchain");

        vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());

        m_image_format = surfaceFormat.format;
        m_extent = extent;

        m_image_views.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; ++i)
        {
            VkImageViewCreateInfo vi{};
            vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            vi.image = m_images[i];
            vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
            vi.format = m_image_format;
            vi.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            vi.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            vi.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            vi.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vi.subresourceRange.baseMipLevel = 0;
            vi.subresourceRange.levelCount = 1;
            vi.subresourceRange.baseArrayLayer = 0;
            vi.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device, &vi, nullptr, &m_image_views[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create image view");
        }
    }

    void Swapchain::recreate(const PhysicalDevice &phys, GLFWwindow *window)
    {
        cleanup();
        create(phys, window);
    }

} // namespace ankh
