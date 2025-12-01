// src/commands/command-buffer.hpp
#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class CommandBuffer
    {
    public:
        CommandBuffer(VkDevice device, VkCommandPool pool);
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer &) = delete;
        CommandBuffer &operator=(const CommandBuffer &) = delete;

        CommandBuffer(CommandBuffer &&) noexcept;
        CommandBuffer &operator=(CommandBuffer &&) noexcept;

        VkCommandBuffer handle() const { return m_buffer; }

        /**
         * Internally uses VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT by default
         */
        void begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        void end();
        void reset(VkCommandBufferResetFlags flags = 0);

    private:
        VkDevice m_device{VK_NULL_HANDLE};
        VkCommandPool m_pool{VK_NULL_HANDLE};
        VkCommandBuffer m_buffer{VK_NULL_HANDLE};
    };

} // namespace ankh
