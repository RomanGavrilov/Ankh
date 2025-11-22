#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class CommandPool
    {
    public:
        CommandPool();
        ~CommandPool();

        VkCommandPool handle() const;
    };

} // namespace ankh