#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class CommandPool
    {
    public:
        CommandPool(VkDevice device, uint32_t queueFamilyIndex);
        ~CommandPool();

        VkCommandPool handle() const;

    private:
        VkDevice m_device{};
        VkCommandPool m_pool{};
    };

} // namespace ankh
