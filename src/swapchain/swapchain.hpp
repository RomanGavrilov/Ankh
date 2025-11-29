// src/swapchain/swapchain.hpp
#pragma once

#include "utils/types.hpp"
#include "renderpass/frame-buffer.hpp"

#include <vector>

struct GLFWwindow;

namespace ankh
{

    class PhysicalDevice;

    class Swapchain
    {
    public:
        Swapchain(const PhysicalDevice &physicalDevice,
                  VkDevice device,
                  VkSurfaceKHR surface,
                  GLFWwindow *window);

        ~Swapchain();

        Swapchain(const Swapchain &) = delete;
        Swapchain &operator=(const Swapchain &) = delete;

        Swapchain(Swapchain &&other) noexcept;
        Swapchain &operator=(Swapchain &&other) noexcept;

        VkSwapchainKHR handle() const { return m_swapchain; }
        VkFormat image_format() const { return m_image_format; }
        VkExtent2D extent() const { return m_extent; }

        const std::vector<VkImageView> &image_views() const { return m_image_views; }

        // Framebuffers for each swapchain image
        const std::vector<Framebuffer> &framebuffers() const { return m_framebuffers; }
        const Framebuffer &framebuffer(uint32_t index) const { return m_framebuffers.at(index); }

        // Called after render pass exists / changes
        void create_framebuffers(VkRenderPass renderPass);
        void destroy_framebuffers();

    private:
        void create_swapchain(const PhysicalDevice &physicalDevice,
                              VkSurfaceKHR surface,
                              GLFWwindow *window);

        void create_image_views();

        // Helpers
        struct SwapChainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice phys, VkSurfaceKHR surface) const;
        VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR> &available) const;
        VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR> &available) const;
        VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR &caps,
                                 GLFWwindow *window) const;

    private:
        VkDevice m_device{VK_NULL_HANDLE};
        VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
        VkFormat m_image_format{};
        VkExtent2D m_extent{};

        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_image_views;

        // New: framebuffer per swapchain image
        std::vector<Framebuffer> m_framebuffers;
    };

} // namespace ankh
