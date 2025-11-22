#pragma once
#include "utils/types.hpp"
#include <vector>

struct GLFWwindow;

namespace ankh
{

    class PhysicalDevice;

    class Swapchain
    {
    public:
        Swapchain(const PhysicalDevice &phys,
                  VkDevice device,
                  VkSurfaceKHR surface,
                  GLFWwindow *window);
        ~Swapchain();

        void recreate(const PhysicalDevice &phys, GLFWwindow *window);

        VkSwapchainKHR handle() const { return m_swapchain; }
        VkFormat image_format() const { return m_image_format; }
        VkExtent2D extent() const { return m_extent; }
        const std::vector<VkImageView> &image_views() const { return m_image_views; }

    private:
        void create(const PhysicalDevice &phys, GLFWwindow *window);
        void cleanup();

        VkDevice m_device{};
        VkSurfaceKHR m_surface{};
        VkSwapchainKHR m_swapchain{};
        VkFormat m_image_format{};
        VkExtent2D m_extent{};

        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_image_views;
    };

} // namespace ankh
