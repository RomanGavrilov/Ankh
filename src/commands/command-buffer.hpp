#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class CommandBuffer
    {
    public:
        CommandBuffer();
        ~CommandBuffer();

        VkCommandBuffer handle() const { return m_cb; }

    private:
        VkCommandBuffer m_cb{};
    };

} // namespace ankh
