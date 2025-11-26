// src/memory/buffer.cpp
#include "memory/buffer.hpp"

#include <stdexcept>

namespace ankh
{

    static uint32_t find_memory_type(VkPhysicalDevice phys,
                                     uint32_t typeFilter,
                                     VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProps{};
        vkGetPhysicalDeviceMemoryProperties(phys, &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1u << i)) &&
                (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type");
    }

    Buffer::Buffer(VkPhysicalDevice phys,
                   VkDevice device,
                   VkDeviceSize size,
                   VkBufferUsageFlags usage,
                   VkMemoryPropertyFlags properties)
        : m_device(device), m_size(size)
    {
        VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bi.size = size;
        bi.usage = usage;
        bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device, &bi, nullptr, &m_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer");
        }

        VkMemoryRequirements req{};
        vkGetBufferMemoryRequirements(m_device, m_buffer, &req);

        VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = find_memory_type(phys, req.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device, &ai, nullptr, &m_memory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate buffer memory");
        }

        vkBindBufferMemory(m_device, m_buffer, m_memory, 0);
    }

    Buffer::~Buffer()
    {
        destroy();
    }

    Buffer::Buffer(Buffer &&other) noexcept
    {
        *this = std::move(other);
    }

    Buffer &Buffer::operator=(Buffer &&other) noexcept
    {
        if (this == &other)
            return *this;

        destroy();

        m_device = other.m_device;
        m_buffer = other.m_buffer;
        m_memory = other.m_memory;
        m_size = other.m_size;

        other.m_device = VK_NULL_HANDLE;
        other.m_buffer = VK_NULL_HANDLE;
        other.m_memory = VK_NULL_HANDLE;
        other.m_size = 0;

        return *this;
    }

    void *Buffer::map(VkDeviceSize offset, VkDeviceSize size)
    {
        void *data = nullptr;
        if (vkMapMemory(m_device, m_memory, offset, size, 0, &data) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to map buffer memory");
        }
        return data;
    }

    void Buffer::unmap()
    {
        if (m_memory)
        {
            vkUnmapMemory(m_device, m_memory);
        }
    }

    void Buffer::destroy()
    {
        if (m_buffer)
        {
            vkDestroyBuffer(m_device, m_buffer, nullptr);
            m_buffer = VK_NULL_HANDLE;
        }
        if (m_memory)
        {
            vkFreeMemory(m_device, m_memory, nullptr);
            m_memory = VK_NULL_HANDLE;
        }
    }

} // namespace ankh
