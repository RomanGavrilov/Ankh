// src/memory/texture.cpp
#include "memory/texture.hpp"

#include "utils/logging.hpp"
#include <stdexcept>

namespace ankh
{
    Texture::Texture(VmaAllocator allocator,
                     VkDevice device,
                     uint32_t width,
                     uint32_t height,
                     VkFormat format,
                     VkImageUsageFlags usage,
                     VmaMemoryUsage memoryUsage,
                     VkImageAspectFlags aspectMask,
                     VkFilter filter,
                     VkSamplerAddressMode addressMode)
        : m_device(device)
        , m_image(allocator, device, width, height, format, usage, memoryUsage, aspectMask)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = filter;
        samplerInfo.minFilter = filter;

        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        ANKH_VK_CHECK(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler));
    }

    Texture::~Texture()
    {
        destroy();
    }

    void Texture::destroy()
    {
        if (m_device == VK_NULL_HANDLE)
            return;

        if (m_sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(m_device, m_sampler, nullptr);
            m_sampler = VK_NULL_HANDLE;
        }

        // m_image destroys itself (vmaDestroyImage + view)
        m_device = VK_NULL_HANDLE;
    }

    Texture::Texture(Texture &&other) noexcept
        : m_device(other.m_device)
        , m_image(std::move(other.m_image))
        , m_sampler(other.m_sampler)
    {
        other.m_device = VK_NULL_HANDLE;
        other.m_sampler = VK_NULL_HANDLE;
    }

    Texture &Texture::operator=(Texture &&other) noexcept
    {
        if (this == &other)
            return *this;

        destroy();

        m_device = other.m_device;
        m_image = std::move(other.m_image);
        m_sampler = other.m_sampler;

        other.m_device = VK_NULL_HANDLE;
        other.m_sampler = VK_NULL_HANDLE;
        return *this;
    }

    VkImage Texture::image() const
    {
        return m_image.image();
    }

    VkImageView Texture::view() const
    {
        return m_image.view();
    }

    VkSampler Texture::sampler() const
    {
        return m_sampler;
    }

    VkFormat Texture::format() const
    {
        return m_image.format();
    }

    uint32_t Texture::width() const
    {
        return m_image.width();
    }

    uint32_t Texture::height() const
    {
        return m_image.height();
    }

    Image &Texture::image_object()
    {
        return m_image;
    }

    const Image &Texture::image_object() const
    {
        return m_image;
    }

} // namespace ankh
