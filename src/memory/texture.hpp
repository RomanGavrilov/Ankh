// src/memory/texture.hpp
#pragma once

#include "image.hpp"
#include "utils/gpu-retirement-queue.hpp"
#include "utils/gpu-signal.hpp"
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

        VkImage image() const;

        VkImageView view() const;

        VkSampler sampler() const;

        VkFormat format() const;

        uint32_t width() const;

        uint32_t height() const;

        Image &image_object();

        const Image &image_object() const;

        void set_retirement(GpuRetirementQueue *q, GpuSignal s) noexcept;

      private:
        void destroy();

        GpuRetirementQueue *m_retirement{nullptr};

        GpuSignal m_signal{};

        VkDevice m_device{VK_NULL_HANDLE};

        Image m_image;

        VkSampler m_sampler{VK_NULL_HANDLE};
    };
} // namespace ankh
