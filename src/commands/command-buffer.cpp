// src/commands/command-buffer.cpp
#include "commands/command-buffer.hpp"

#include <stdexcept>

namespace ankh
{

    CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool pool)
        : m_device(device), m_pool(pool)
    {
        VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        ai.commandPool = m_pool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_device, &ai, &m_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffer");
        }
    }

    CommandBuffer::~CommandBuffer()
    {
        if (m_buffer)
        {
            vkFreeCommandBuffers(m_device, m_pool, 1, &m_buffer);
            m_buffer = VK_NULL_HANDLE;
        }
    }

    CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept
    {
        *this = std::move(other);
    }

    CommandBuffer &CommandBuffer::operator=(CommandBuffer &&other) noexcept
    {
        if (this == &other)
            return *this;

        if (m_buffer)
        {
            vkFreeCommandBuffers(m_device, m_pool, 1, &m_buffer);
        }

        m_device = other.m_device;
        m_pool = other.m_pool;
        m_buffer = other.m_buffer;

        other.m_device = VK_NULL_HANDLE;
        other.m_pool = VK_NULL_HANDLE;
        other.m_buffer = VK_NULL_HANDLE;

        return *this;
    }

    void CommandBuffer::begin(VkCommandBufferUsageFlags flags)
    {
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        bi.flags = flags;

        if (vkBeginCommandBuffer(m_buffer, &bi) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin command buffer");
        }
    }

    void CommandBuffer::end()
    {
        if (vkEndCommandBuffer(m_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to end command buffer");
        }
    }

    void CommandBuffer::reset(VkCommandBufferResetFlags flags)
    {
        if (vkResetCommandBuffer(m_buffer, flags) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to reset command buffer");
        }
    }

} // namespace ankh
