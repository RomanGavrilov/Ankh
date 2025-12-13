#include "memory/buffer.hpp"
#include "utils/logging.hpp"
#include <stdexcept>

namespace ankh
{
    Buffer::Buffer(VmaAllocator allocator,
                   VkDevice device,
                   VkDeviceSize size,
                   VkBufferUsageFlags usage,
                   VmaMemoryUsage memoryUsage,
                   VmaAllocationCreateFlags allocFlags)
        : m_allocator(allocator)
        , m_device(device)
        , m_size(size)
    {
        VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bi.size = size;
        bi.usage = usage;
        bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo ai{};
        ai.usage = memoryUsage;
        ai.flags = allocFlags;

        ANKH_VK_CHECK(vmaCreateBuffer(m_allocator, &bi, &ai, &m_buffer, &m_allocation, nullptr));
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
        {
            return *this;
        }

        destroy();

        m_allocator = other.m_allocator;
        m_device = other.m_device;
        m_buffer = other.m_buffer;
        m_allocation = other.m_allocation;
        m_size = other.m_size;
        m_mapped = other.m_mapped;

        other.m_allocator = VK_NULL_HANDLE;
        other.m_device = VK_NULL_HANDLE;
        other.m_buffer = VK_NULL_HANDLE;
        other.m_allocation = VK_NULL_HANDLE;
        other.m_size = 0;
        other.m_mapped = nullptr;

        return *this;
    }

    void Buffer::destroy()
    {
        if (m_mapped && m_allocator && m_allocation)
        {
            vmaUnmapMemory(m_allocator, m_allocation);
            m_mapped = nullptr;
        }

        if (m_allocator && m_buffer && m_allocation)
        {
            vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
            m_buffer = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
        }
    }

    void *Buffer::map()
    {
        if (m_mapped)
        {
            return m_mapped;
        }

        void *ptr = nullptr;

        ANKH_VK_CHECK(vmaMapMemory(m_allocator, m_allocation, &ptr));

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
