#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class CommandPool
    {
    public:
        CommandPool(VkDevice device, uint32_t queueFamilyIndex);
        CommandPool(const CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;
        ~CommandPool();

        VkCommandPool handle() const;

    private:
        VkDevice m_device{};
        VkCommandPool m_pool{};
    };

} // namespace ankh
