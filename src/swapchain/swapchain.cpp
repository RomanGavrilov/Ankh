// src/swapchain/swapchain.cpp

#include "swapchain/swapchain.hpp"

#include "core/physical-device.hpp"
#include "renderpass/frame-buffer.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <stdexcept>

namespace ankh
{

    Swapchain::Swapchain(const PhysicalDevice &physicalDevice,
                         VkDevice device,
                         VkSurfaceKHR surface,
                         GLFWwindow *window)
        : m_device(device)
    {
        create_swapchain(physicalDevice, surface, window);
        create_image_views();
    }

    Swapchain::~Swapchain()
    {
        destroy_framebuffers();

        for (auto view : m_image_views)
        {
            vkDestroyImageView(m_device, view, nullptr);
        }
        m_image_views.clear();

        if (m_swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
    }

    Swapchain::Swapchain(Swapchain &&other) noexcept
    {
        *this = std::move(other);
    }

    Swapchain &Swapchain::operator=(Swapchain &&other) noexcept
    {
        if (this == &other)
            return *this;

        // Destroy current
        this->~Swapchain();

        m_device = other.m_device;
        m_swapchain = other.m_swapchain;
        m_image_format = other.m_image_format;
        m_extent = other.m_extent;
        m_images = std::move(other.m_images);
        m_image_views = std::move(other.m_image_views);
        m_framebuffers = std::move(other.m_framebuffers);

        other.m_device = VK_NULL_HANDLE;
        other.m_swapchain = VK_NULL_HANDLE;
        other.m_image_views.clear();
        other.m_images.clear();
        other.m_framebuffers.clear();

        return *this;
    }

    void Swapchain::create_swapchain(const PhysicalDevice &physicalDevice,
                                     VkSurfaceKHR surface,
                                     GLFWwindow *window)
    {
        VkPhysicalDevice phys = physicalDevice.handle();

        SwapChainSupportDetails support = query_swapchain_support(phys, surface);

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
        ci.surface = surface;
        ci.minImageCount = imageCount;
        ci.imageFormat = surfaceFormat.format;
        ci.imageColorSpace = surfaceFormat.colorSpace;
        ci.imageExtent = extent;
        ci.imageArrayLayers = 1;
        ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = physicalDevice.queues();
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
            ci.queueFamilyIndexCount = 0;
            ci.pQueueFamilyIndices = nullptr;
        }

        ci.preTransform = support.capabilities.currentTransform;
        ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        ci.presentMode = presentMode;
        ci.clipped = VK_TRUE;
        ci.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_device, &ci, nullptr, &m_swapchain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swapchain");
        }

        // Fetch images
        uint32_t actualImageCount = 0;
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount, nullptr);
        m_images.resize(actualImageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount, m_images.data());

        m_image_format = surfaceFormat.format;
        m_extent = extent;
    }

    void Swapchain::create_image_views()
    {
        m_image_views.resize(m_images.size());

        for (size_t i = 0; i < m_images.size(); ++i)
        {
            VkImageViewCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ci.image = m_images[i];
            ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ci.format = m_image_format;

            ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            ci.subresourceRange.baseMipLevel = 0;
            ci.subresourceRange.levelCount = 1;
            ci.subresourceRange.baseArrayLayer = 0;
            ci.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device, &ci, nullptr, &m_image_views[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create swapchain image view");
            }
        }
    }

    void Swapchain::create_framebuffers(VkRenderPass renderPass)
    {
        m_framebuffers.clear();
        m_framebuffers.reserve(m_image_views.size());

        for (auto view : m_image_views)
        {
            m_framebuffers.emplace_back(
                m_device,
                renderPass,
                view,
                m_extent);
        }
    }

    void Swapchain::destroy_framebuffers()
    {
        m_framebuffers.clear(); // Framebuffer RAII will destroy VkFramebuffer
    }

    // ==== helpers ====

    Swapchain::SwapChainSupportDetails
    Swapchain::query_swapchain_support(VkPhysicalDevice phys, VkSurfaceKHR surface) const
    {
        SwapChainSupportDetails details{};

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys, surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(phys, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(phys, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR
    Swapchain::choose_surface_format(const std::vector<VkSurfaceFormatKHR> &available) const
    {
        for (const auto &f : available)
        {
            if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
                f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return f;
            }
        }

        return available[0];
    }

    VkPresentModeKHR
    Swapchain::choose_present_mode(const std::vector<VkPresentModeKHR> &available) const
    {
        for (const auto &m : available)
        {
            if (m == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return m;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D
    Swapchain::choose_extent(const VkSurfaceCapabilitiesKHR &caps,
                             GLFWwindow *window) const
    {
        if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return caps.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actual{};
            actual.width = static_cast<uint32_t>(width);
            actual.height = static_cast<uint32_t>(height);

            actual.width = std::clamp(actual.width, caps.minImageExtent.width, caps.maxImageExtent.width);
            actual.height = std::clamp(actual.height, caps.minImageExtent.height, caps.maxImageExtent.height);

            return actual;
        }
    }

} // namespace ankh
