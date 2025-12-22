#pragma once

#include "utils/types.hpp"
#include <vk_mem_alloc.h>

namespace ankh
{
    // GPU-only device local buffer
    class Buffer
    {
      public:
        Buffer() = delete;

        Buffer(VmaAllocator allocator,
               VkDevice device,
               VkDeviceSize size,
               VkBufferUsageFlags usage,
               VmaMemoryUsage memoryUsage,
               VmaAllocationCreateFlags allocFlags = 0);

        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        Buffer(Buffer &&other) noexcept;
        Buffer &operator=(Buffer &&other) noexcept;

        VkBuffer handle() const
        {
            return m_buffer;
        }
        VkDevice device() const
        {
            return m_device;
        }
        VkDeviceSize size() const
        {
            return m_size;
        }

        void *map();
        void unmap();

      private:
        void destroy();

        VkDevice m_device{VK_NULL_HANDLE};
        VmaAllocator m_allocator{VK_NULL_HANDLE};
        VkBuffer m_buffer{VK_NULL_HANDLE};
        VmaAllocation m_allocation{VK_NULL_HANDLE};

        VkDeviceSize m_size{0};
        void *m_mapped{nullptr};
    };
} // namespace ankh
