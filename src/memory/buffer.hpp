// src/memory/buffer.hpp
#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class Buffer
    {
    public:
        Buffer(VkPhysicalDevice phys,
               VkDevice device,
               VkDeviceSize size,
               VkBufferUsageFlags usage,
               VkMemoryPropertyFlags props);
        ~Buffer();

        VkBuffer handle() const { return m_buffer; }
        VkDeviceSize size() const { return m_size; }

        // Map/unmap for HOST_VISIBLE buffers
        void *map(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
        void unmap();

    private:
        VkDevice m_device{};
        VkBuffer m_buffer{};
        VkDeviceMemory m_memory{};
        VkDeviceSize m_size{};
    };

} // namespace ankh
