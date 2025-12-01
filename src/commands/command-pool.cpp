// src/commands/command-pool.cpp
#include "commands/command-pool.hpp"
#include <stdexcept>

namespace ankh
{

    CommandPool::CommandPool(VkDevice device, uint32_t queueFamilyIndex)
        : m_device(device)
    {
        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = queueFamilyIndex;

        if (vkCreateCommandPool(m_device, &info, nullptr, &m_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool");
        }
    }

    CommandPool::~CommandPool()
    {
        if (m_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(m_device, m_pool, nullptr);
            m_pool = VK_NULL_HANDLE;
        }
    }

    CommandPool::CommandPool(CommandPool &&other) noexcept
        : m_device(other.m_device)
        , m_pool(other.m_pool)
    {
        // mark other as "empty" so its destructor does nothing
        other.m_device = VK_NULL_HANDLE;
        other.m_pool = VK_NULL_HANDLE;
    }

    CommandPool &CommandPool::operator=(CommandPool &&other) noexcept
    {
        if (this == &other)
            return *this;

        // Destroy our current pool (if any)
        if (m_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(m_device, m_pool, nullptr);
        }

        // Steal other's pool
        m_device = other.m_device;
        m_pool = other.m_pool;

        // Null out other so its destructor is inert
        other.m_device = VK_NULL_HANDLE;
        other.m_pool = VK_NULL_HANDLE;

        return *this;
    }

} // namespace ankh
