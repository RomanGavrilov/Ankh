#pragma once

#include "utils/gpu-retirement-queue.hpp"
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

        Buffer(VmaAllocator allocator,
               VkDevice device,
               VkDeviceSize size,
               VkBufferUsageFlags usage,
               VmaMemoryUsage memoryUsage,
               GpuRetirementQueue *retirement,
               GpuSignal signal,
               VmaAllocationCreateFlags allocFlags = 0);

        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        Buffer(Buffer &&other) noexcept;
        Buffer &operator=(Buffer &&other) noexcept;

        VkBuffer handle() const;
        VkDevice device() const;
        VkDeviceSize size() const;

        void set_retirement(GpuRetirementQueue *retirement, GpuSignal signal) noexcept
        {
            m_retirement = retirement;
            m_signal = signal;
        }

        void *map();
        void unmap();

      private:
        void destroy();

      private:
        GpuRetirementQueue *m_retirement{nullptr};

        GpuSignal m_signal{};

        VkDevice m_device;

        VmaAllocator m_allocator;

        VkBuffer m_buffer;

        VmaAllocation m_allocation;

        VkDeviceSize m_size;

        void *m_mapped{nullptr};
    };
} // namespace ankh
