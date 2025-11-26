// src/memory/buffer.hpp
#pragma once

#include "utils/types.hpp"

namespace ankh
{

    class Buffer
    {
    public:
        Buffer() = delete;

        Buffer(VkPhysicalDevice phys,
               VkDevice device,
               VkDeviceSize size,
               VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties);

        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        Buffer(Buffer &&other) noexcept;
        Buffer &operator=(Buffer &&other) noexcept;

        VkBuffer handle() const { return m_buffer; }
        VkDeviceMemory memory() const { return m_memory; }
        VkDevice device() const { return m_device; }
        VkDeviceSize size() const { return m_size; }

        void *map(VkDeviceSize offset, VkDeviceSize size);
        void unmap();

    private:
        void destroy();

        VkDevice m_device{VK_NULL_HANDLE};
        VkBuffer m_buffer{VK_NULL_HANDLE};
        VkDeviceMemory m_memory{VK_NULL_HANDLE};
        VkDeviceSize m_size{0};
    };

} // namespace ankh
