// src/memory/image.hpp
#pragma once

#include "utils/types.hpp"

namespace ankh
{

    // Simple 2D image wrapper (VkImage + VkDeviceMemory + VkImageView).
    // Designed for textures or color attachments with a single mip level.
    class Image
    {
      public:
        Image() = delete;

        Image(VkPhysicalDevice physicalDevice,
              VkDevice device,
              uint32_t width,
              uint32_t height,
              VkFormat format,
              VkImageUsageFlags usage,
              VkMemoryPropertyFlags memoryProperties,
              VkImageAspectFlags aspectMask);

        ~Image();

        Image(const Image &) = delete;
        Image &operator=(const Image &) = delete;

        Image(Image &&other) noexcept;
        Image &operator=(Image &&other) noexcept;

        VkImage image() const { return m_image; }
        VkImageView view() const { return m_view; }
        VkDevice device() const { return m_device; }
        VkFormat format() const { return m_format; }
        uint32_t width() const { return m_width; }
        uint32_t height() const { return m_height; }

      private:
        void destroy();

        VkDevice m_device{VK_NULL_HANDLE};
        VkImage m_image{VK_NULL_HANDLE};
        VkDeviceMemory m_memory{VK_NULL_HANDLE};
        VkImageView m_view{VK_NULL_HANDLE};

        VkFormat m_format{};
        uint32_t m_width{0};
        uint32_t m_height{0};
    };

} // namespace ankh
