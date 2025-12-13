// src/memory/image.cpp
#include "memory/image.hpp"
#include "utils/logging.hpp"

#include <stdexcept>

namespace ankh
{
    Image::Image(VmaAllocator allocator,
                 VkDevice device,
                 uint32_t width,
                 uint32_t height,
                 VkFormat format,
                 VkImageUsageFlags usage,
                 VmaMemoryUsage memoryUsage,
                 VkImageAspectFlags aspectMask)
        : m_allocator(allocator)
        , m_device(device)
        , m_format(format)
        , m_width(width)
        , m_height(height)
    {
        if (!m_allocator || m_device == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Image: invalid allocator/device");
        }

        VkImageCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ci.imageType = VK_IMAGE_TYPE_2D;
        ci.extent.width = width;
        ci.extent.height = height;
        ci.extent.depth = 1;
        ci.mipLevels = 1;
        ci.arrayLayers = 1;
        ci.format = format;
        ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ci.usage = usage;
        ci.samples = VK_SAMPLE_COUNT_1_BIT;
        ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo ai{};
        ai.usage = memoryUsage;

        VkResult res = vmaCreateImage(m_allocator, &ci, &ai, &m_image, &m_allocation, nullptr);
        if (res != VK_SUCCESS)
        {
            ANKH_LOG_ERROR("[Image] vmaCreateImage failed");
            throw std::runtime_error("vmaCreateImage failed");
        }

        VkImageViewCreateInfo vi{};
        vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vi.image = m_image;
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.format = format;
        vi.subresourceRange.aspectMask = aspectMask;
        vi.subresourceRange.baseMipLevel = 0;
        vi.subresourceRange.levelCount = 1;
        vi.subresourceRange.baseArrayLayer = 0;
        vi.subresourceRange.layerCount = 1;

        ANKH_VK_CHECK(vkCreateImageView(m_device, &vi, nullptr, &m_view));
    }

    Image::~Image()
    {
        destroy();
    }

    void Image::destroy()
    {
        if (m_device == VK_NULL_HANDLE)
            return;

        if (m_view != VK_NULL_HANDLE)
        {
            vkDestroyImageView(m_device, m_view, nullptr);
            m_view = VK_NULL_HANDLE;
        }

        if (m_allocator && m_image != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE)
        {
            vmaDestroyImage(m_allocator, m_image, m_allocation);
            m_image = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
        }

        m_device = VK_NULL_HANDLE;
        m_allocator = VK_NULL_HANDLE;
        m_width = 0;
        m_height = 0;
        m_format = {};
    }

    Image::Image(Image &&other) noexcept
    {
        *this = std::move(other);
    }

    Image &Image::operator=(Image &&other) noexcept
    {
        if (this == &other)
            return *this;

        destroy();

        m_allocator = other.m_allocator;
        m_device = other.m_device;
        m_image = other.m_image;
        m_allocation = other.m_allocation;
        m_view = other.m_view;
        m_format = other.m_format;
        m_width = other.m_width;
        m_height = other.m_height;

        other.m_allocator = VK_NULL_HANDLE;
        other.m_device = VK_NULL_HANDLE;
        other.m_image = VK_NULL_HANDLE;
        other.m_allocation = VK_NULL_HANDLE;
        other.m_view = VK_NULL_HANDLE;
        other.m_format = {};
        other.m_width = 0;
        other.m_height = 0;

        return *this;
    }

} // namespace ankh
