// src/memory/image.cpp
#include "memory/image.hpp"

#include <stdexcept>

namespace ankh
{
    namespace
    {
        uint32_t find_memory_type(VkPhysicalDevice phys, uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProps{};
            vkGetPhysicalDeviceMemoryProperties(phys, &memProps);

            for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
            {
                if ((typeFilter & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable image memory type");
        }
    } // namespace

    Image::Image(VkPhysicalDevice physicalDevice,
                 VkDevice device,
                 uint32_t width,
                 uint32_t height,
                 VkFormat format,
                 VkImageUsageFlags usage,
                 VkMemoryPropertyFlags memoryProperties,
                 VkImageAspectFlags aspectMask)
        : m_device(device)
        , m_format(format)
        , m_width(width)
        , m_height(height)
    {
        // Create VkImage
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(m_device, &imageInfo, nullptr, &m_image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image");
        }

        // Allocate memory
        VkMemoryRequirements memReq{};
        vkGetImageMemoryRequirements(m_device, m_image, &memReq);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = find_memory_type(physicalDevice, memReq.memoryTypeBits, memoryProperties);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_memory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory");
        }

        if (vkBindImageMemory(m_device, m_image, m_memory, 0) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to bind image memory");
        }

        // Create image view
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_format;
        viewInfo.subresourceRange.aspectMask = aspectMask;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_view) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image view");
        }
    }

    Image::~Image() { destroy(); }

    void Image::destroy()
    {
        if (m_device == VK_NULL_HANDLE)
            return;

        if (m_view != VK_NULL_HANDLE)
        {
            vkDestroyImageView(m_device, m_view, nullptr);
            m_view = VK_NULL_HANDLE;
        }

        if (m_image != VK_NULL_HANDLE)
        {
            vkDestroyImage(m_device, m_image, nullptr);
            m_image = VK_NULL_HANDLE;
        }

        if (m_memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(m_device, m_memory, nullptr);
            m_memory = VK_NULL_HANDLE;
        }

        m_device = VK_NULL_HANDLE;
    }

    Image::Image(Image &&other) noexcept
        : m_device(other.m_device)
        , m_image(other.m_image)
        , m_memory(other.m_memory)
        , m_view(other.m_view)
        , m_format(other.m_format)
        , m_width(other.m_width)
        , m_height(other.m_height)
    {
        other.m_device = VK_NULL_HANDLE;
        other.m_image = VK_NULL_HANDLE;
        other.m_memory = VK_NULL_HANDLE;
        other.m_view = VK_NULL_HANDLE;
        other.m_width = 0;
        other.m_height = 0;
    }

    Image &Image::operator=(Image &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        destroy();

        m_device = other.m_device;
        m_image = other.m_image;
        m_memory = other.m_memory;
        m_view = other.m_view;
        m_format = other.m_format;
        m_width = other.m_width;
        m_height = other.m_height;

        other.m_device = VK_NULL_HANDLE;
        other.m_image = VK_NULL_HANDLE;
        other.m_memory = VK_NULL_HANDLE;
        other.m_view = VK_NULL_HANDLE;
        other.m_width = 0;
        other.m_height = 0;

        return *this;
    }

} // namespace ankh
