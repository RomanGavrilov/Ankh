// src/commands/command-pool.cpp
#include "commands/command-pool.hpp"

#include <stdexcept>

namespace ankh
{

    CommandPool::CommandPool(VkDevice device, uint32_t queueFamilyIndex)
        : m_device(device)
    {
        VkCommandPoolCreateInfo ci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        ci.queueFamilyIndex = queueFamilyIndex;

        if (vkCreateCommandPool(m_device, &ci, nullptr, &m_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool");
        }
    }

    CommandPool::~CommandPool()
    {
        if (m_pool)
        {
            vkDestroyCommandPool(m_device, m_pool, nullptr);
            m_pool = VK_NULL_HANDLE;
        }
    }

    CommandPool::CommandPool(CommandPool &&other) noexcept
    {
        *this = std::move(other);
    }

    CommandPool &CommandPool::operator=(CommandPool &&other) noexcept
    {
        if (&other == this)
        {
            return *this;
        }

        if (m_pool)
        {
            vkDestroyCommandPool(m_device, m_pool, nullptr);
        }

        m_device = other.m_device;
        m_pool = other.m_pool;

        other.m_device = VK_NULL_HANDLE;
        other.m_pool = VK_NULL_HANDLE;

        return *this;
    }

} // namespace ankh
