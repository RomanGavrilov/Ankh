// src/commands/command-pool.hpp
#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class CommandPool
    {
      public:
        CommandPool(VkDevice device, uint32_t queueFamilyIndex);
        ~CommandPool();

        CommandPool(const CommandPool &) = delete;
        CommandPool &operator=(const CommandPool &) = delete;

        CommandPool(CommandPool &&) noexcept;
        CommandPool &operator=(CommandPool &&) noexcept;

        VkCommandPool handle() const { return m_pool; }
        VkDevice device() const { return m_device; }

      private:
        VkDevice m_device{VK_NULL_HANDLE};
        VkCommandPool m_pool{VK_NULL_HANDLE};
    };

} // namespace ankh
