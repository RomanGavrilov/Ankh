#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class CommandBuffer
    {
    public:
        CommandBuffer();
        ~CommandBuffer();

        VkCommandBuffer handle() const;
    };

} // namespace ankh