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

        VkBuffer handle() const;
        VkDevice device() const;
        VkDeviceSize size() const;
        
        void *map();
        void unmap();

      private:
        void destroy();

        VkDevice m_device;
        
        VmaAllocator m_allocator;
        
        VkBuffer m_buffer;

        VmaAllocation m_allocation;

        VkDeviceSize m_size;
        
        void *m_mapped;
    };
} // namespace ankh
