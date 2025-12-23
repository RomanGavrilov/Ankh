#include "memory/buffer.hpp"
#include "utils/logging.hpp"
#include <stdexcept>

namespace ankh
{
    namespace
    {
        static void create_buffer(VmaAllocator allocator,
                                  VkDeviceSize size,
                                  VkBufferUsageFlags usage,
                                  VmaMemoryUsage memoryUsage,
                                  VmaAllocationCreateFlags allocFlags,
                                  VkBuffer &outBuffer,
                                  VmaAllocation &outAllocation)
        {
            VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = memoryUsage;
            allocInfo.flags = allocFlags;

            ANKH_VK_CHECK(vmaCreateBuffer(allocator,
                                          &bufferInfo,
                                          &allocInfo,
                                          &outBuffer,
                                          &outAllocation,
                                          nullptr));
        }
    } // namespace

    Buffer::Buffer(VmaAllocator allocator,
                   VkDevice device,
                   VkDeviceSize size,
                   VkBufferUsageFlags usage,
                   VmaMemoryUsage memoryUsage,
                   VmaAllocationCreateFlags allocFlags)
        : m_device(device)
        , m_allocator(allocator)
        , m_size(size)
        , m_retirement(nullptr)
        , m_signal{}
    {
        create_buffer(m_allocator, m_size, usage, memoryUsage, allocFlags, m_buffer, m_allocation);
    }

    Buffer::Buffer(VmaAllocator allocator,
                   VkDevice device,
                   VkDeviceSize size,
                   VkBufferUsageFlags usage,
                   VmaMemoryUsage memoryUsage,
                   GpuRetirementQueue *retirement,
                   GpuSignal signal,
                   VmaAllocationCreateFlags allocFlags)
        : m_device(device)
        , m_allocator(allocator)
        , m_size(size)
        , m_retirement(retirement)
        , m_signal(signal)
    {
        create_buffer(m_allocator, m_size, usage, memoryUsage, allocFlags, m_buffer, m_allocation);
    }

    Buffer::~Buffer()
    {
        destroy();
    }

    Buffer::Buffer(Buffer &&other) noexcept
        : m_device(std::exchange(other.m_device, VK_NULL_HANDLE))
        , m_allocator(std::exchange(other.m_allocator, VK_NULL_HANDLE))
        , m_buffer(std::exchange(other.m_buffer, VK_NULL_HANDLE))
        , m_allocation(std::exchange(other.m_allocation, VK_NULL_HANDLE))
        , m_size(std::exchange(other.m_size, 0))
        , m_mapped(std::exchange(other.m_mapped, nullptr))
        , m_retirement(std::exchange(other.m_retirement, nullptr))
        , m_signal(std::exchange(other.m_signal, GpuSignal{}))
    {
    }

    Buffer &Buffer::operator=(Buffer &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        destroy();

        m_device = std::exchange(other.m_device, VK_NULL_HANDLE);
        m_allocator = std::exchange(other.m_allocator, VK_NULL_HANDLE);
        m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
        m_allocation = std::exchange(other.m_allocation, VK_NULL_HANDLE);
        m_size = std::exchange(other.m_size, 0);
        m_mapped = std::exchange(other.m_mapped, nullptr);
        m_retirement = std::exchange(other.m_retirement, nullptr);
        m_signal = std::exchange(other.m_signal, GpuSignal{});

        return *this;
    }

    VkBuffer Buffer::handle() const
    {
        return m_buffer;
    }

    VkDevice Buffer::device() const
    {
        return m_device;
    }

    VkDeviceSize Buffer::size() const
    {
        return m_size;
    }

    void Buffer::destroy()
    {
        if (!m_buffer)
        {
            return;
        }

        if (m_retirement && m_signal.value != 0)
        {
            auto buffer = m_buffer;
            auto allocation = m_allocation;
            auto allocator = m_allocator;
            auto mapped = m_mapped;

            m_retirement->retire_after(m_signal,
                                       [allocator, buffer, allocation, mapped]() mutable
                                       {
                                           if (mapped)
                                           {
                                               vmaUnmapMemory(allocator, allocation);
                                           }
                                           vmaDestroyBuffer(allocator, buffer, allocation);
                                       });
        }
        else
        {
            if (m_mapped)
            {
                vmaUnmapMemory(m_allocator, m_allocation);
            }

            vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
        }

        m_buffer = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
        m_mapped = nullptr;
    }

    void *Buffer::map()
    {
        ANKH_ASSERT(m_buffer != VK_NULL_HANDLE);
        ANKH_ASSERT(m_allocator != VK_NULL_HANDLE);
        ANKH_ASSERT(m_allocation != VK_NULL_HANDLE);

        if (m_mapped)
        {
            return m_mapped;
        }

        void *ptr = nullptr;
        ANKH_VK_CHECK(vmaMapMemory(m_allocator, m_allocation, &ptr));
        ANKH_ASSERT(ptr != nullptr);

        m_mapped = ptr;
        return m_mapped;
    }

    void Buffer::unmap()
    {
        if (!m_mapped)
        {
            return;
        }

        vmaUnmapMemory(m_allocator, m_allocation);

        m_mapped = nullptr;
    }
} // namespace ankh
