// src/memory/buffer.cpp
#include "buffer.hpp"
#include <stdexcept>

namespace ankh
{

    static uint32_t findMemoryType(VkPhysicalDevice phys,
                                   uint32_t typeFilter,
                                   VkMemoryPropertyFlags props)
    {
        VkPhysicalDeviceMemoryProperties mem{};
        vkGetPhysicalDeviceMemoryProperties(phys, &mem);

        for (uint32_t i = 0; i < mem.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1u << i)) &&
                (mem.memoryTypes[i].propertyFlags & props) == props)
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
                   VkMemoryPropertyFlags props)
        : m_device(device), m_size(size)
    {

        VkBufferCreateInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bi.size = size;
        bi.usage = usage;
        bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bi, nullptr, &m_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer");
        }

        VkMemoryRequirements req{};
        vkGetBufferMemoryRequirements(device, m_buffer, &req);

        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(phys, req.memoryTypeBits, props);

        if (vkAllocateMemory(device, &ai, nullptr, &m_memory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate buffer memory");
        }

        vkBindBufferMemory(device, m_buffer, m_memory, 0);
    }

    Buffer::~Buffer()
    {
        if (m_buffer)
        {
            vkDestroyBuffer(m_device, m_buffer, nullptr);
        }
        if (m_memory)
        {
            vkFreeMemory(m_device, m_memory, nullptr);
        }
    }

    void *Buffer::map(VkDeviceSize offset, VkDeviceSize size)
    {
        void *data = nullptr;
        VkResult res = vkMapMemory(m_device, m_memory, offset, size, 0, &data);
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("failed to map buffer memory");
        }
        return data;
    }

    void Buffer::unmap()
    {
        vkUnmapMemory(m_device, m_memory);
    }

} // namespace ankh
