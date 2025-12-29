// src/memory/image.hpp
#pragma once

#include "utils/gpu-retirement-queue.hpp"
#include "utils/gpu-signal.hpp"
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

        VkImage image() const;

        VkImageView view() const;

        VkDevice device() const;

        VkFormat format() const;

        uint32_t width() const;

        uint32_t height() const;

        void set_retirement(GpuRetirementQueue *retirement, GpuSignal signal) noexcept;

      private:
        void destroy();

        GpuRetirementQueue *m_retirement{nullptr};
        GpuSignal m_signal{};

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
