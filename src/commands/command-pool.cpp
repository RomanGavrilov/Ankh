// src/commands/command-pool.cpp
#include "command-pool.hpp"
#include <stdexcept>

namespace ankh
{
    CommandPool::CommandPool(VkDevice device, uint32_t qFamily)
        : m_device(device)
    {
        VkCommandPoolCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        ci.queueFamilyIndex = qFamily;
        if (vkCreateCommandPool(device, &ci, nullptr, &m_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool");
        }
    }

    CommandPool::~CommandPool()
    {
        if (m_pool)
        {
            vkDestroyCommandPool(m_device, m_pool, nullptr);
        }
    }

    VkCommandPool CommandPool::handle() const { return m_pool; }
} // namespace ankh