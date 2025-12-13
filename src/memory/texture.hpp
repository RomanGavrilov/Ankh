// src/memory/texture.hpp
#pragma once

#include "image.hpp"
#include "utils/types.hpp"
#include <vk_mem_alloc.h>

namespace ankh
{
    // Simple 2D texture wrapper: Image + VkSampler.
    // Single mip, single layer, no anisotropy.
    class Texture
    {
      public:
        Texture() = delete;

        Texture(VmaAllocator allocator,
                VkDevice device,
                uint32_t width,
                uint32_t height,
                VkFormat format,
                VkImageUsageFlags usage,
                VmaMemoryUsage memoryUsage,
                VkImageAspectFlags aspectMask,
                VkFilter filter = VK_FILTER_LINEAR,
                VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

        Texture(Texture &&other) noexcept;
        Texture &operator=(Texture &&other) noexcept;

        VkImage image() const
        {
            return m_image.image();
        }
        VkImageView view() const
        {
            return m_image.view();
        }
        VkSampler sampler() const
        {
            return m_sampler;
        }
        VkFormat format() const
        {
            return m_image.format();
        }
        uint32_t width() const
        {
            return m_image.width();
        }
        uint32_t height() const
        {
            return m_image.height();
        }

        Image &image_object()
        {
            return m_image;
        }
        const Image &image_object() const
        {
            return m_image;
        }

      private:
        void destroy();

        VkDevice m_device{VK_NULL_HANDLE};
        Image m_image;
        VkSampler m_sampler{VK_NULL_HANDLE};
    };
} // namespace ankh
