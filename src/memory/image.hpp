// src/memory/image.hpp
#pragma once

#include "utils/types.hpp"
#include <vk_mem_alloc.h>

namespace ankh
{
    // Simple 2D image wrapper (VkImage + VMA allocation + VkImageView).
    // Single mip, single layer. Works for textures and depth images.
    class Image
    {
      public:
        Image() = delete;

        Image(VmaAllocator allocator,
              VkDevice device,
              uint32_t width,
              uint32_t height,
              VkFormat format,
              VkImageUsageFlags usage,
              VmaMemoryUsage memoryUsage,
              VkImageAspectFlags aspectMask);

        ~Image();

        Image(const Image &) = delete;
        Image &operator=(const Image &) = delete;

        Image(Image &&other) noexcept;
        Image &operator=(Image &&other) noexcept;

        VkImage image() const
        {
            return m_image;
        }
        VkImageView view() const
        {
            return m_view;
        }
        VkDevice device() const
        {
            return m_device;
        }
        VkFormat format() const
        {
            return m_format;
        }
        uint32_t width() const
        {
            return m_width;
        }
        uint32_t height() const
        {
            return m_height;
        }

      private:
        void destroy();

        
        VkDevice m_device{VK_NULL_HANDLE};
        VmaAllocator m_allocator{VK_NULL_HANDLE};
        VkImage m_image{VK_NULL_HANDLE};
        VmaAllocation m_allocation{VK_NULL_HANDLE};
        VkImageView m_view{VK_NULL_HANDLE};

        VkFormat m_format{};
        uint32_t m_width{0};
        uint32_t m_height{0};
    };

} // namespace ankh
