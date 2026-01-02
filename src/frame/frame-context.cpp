// src/frame/frame-context.cpp
#include "frame/frame-context.hpp"

#include "commands/command-buffer.hpp"
#include "commands/command-pool.hpp"
#include "descriptors/descriptor-writer.hpp"
#include "memory/buffer.hpp"
#include "utils/gpu-retirement-queue.hpp"

#include <stdexcept>
#include <utils/config.hpp>
#include <utils/logging.hpp>

namespace ankh
{

    FrameContext::FrameContext(VmaAllocator allocator,
                               VkDevice device,
                               uint32_t graphicsQueueFamilyIndex,
                               VkDescriptorSet descriptorSet,
                               VkImageView textureView,
                               VkSampler textureSampler,
                               GpuRetirementQueue *retirement)
        : m_device{device}
        , m_descriptor_set{descriptorSet}
        , m_retirement{retirement}
    {
        // Command pool & buffer
        m_pool = std::make_unique<CommandPool>(m_device, graphicsQueueFamilyIndex);

        m_cmd = std::make_unique<CommandBuffer>(m_device, m_pool->handle());

        m_object_capacity = ankh::config().maxObjects;

        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        ANKH_VK_CHECK(vkCreateSemaphore(m_device, &semInfo, nullptr, &m_image_available));
        ANKH_VK_CHECK(vkCreateSemaphore(m_device, &semInfo, nullptr, &m_render_finished));

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        ANKH_VK_CHECK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_in_flight));
    }

    FrameContext::~FrameContext()
    {
        if (m_device == VK_NULL_HANDLE)
        {
            return;
        }

        if (m_image_available != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device, m_image_available, nullptr);
            m_image_available = VK_NULL_HANDLE;
        }

        if (m_render_finished != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device, m_render_finished, nullptr);
            m_render_finished = VK_NULL_HANDLE;
        }

        if (m_in_flight != VK_NULL_HANDLE)
        {
            vkDestroyFence(m_device, m_in_flight, nullptr);
            m_in_flight = VK_NULL_HANDLE;
        }
    }

    FrameContext::FrameContext(FrameContext &&other) noexcept
        : m_device(other.m_device)
        , m_pool(std::move(other.m_pool))
        , m_cmd(std::move(other.m_cmd))
        , m_descriptor_set(other.m_descriptor_set)
        , m_image_available(other.m_image_available)
        , m_render_finished(other.m_render_finished)
        , m_in_flight(other.m_in_flight)
    {
        other.m_device = VK_NULL_HANDLE;
        other.m_descriptor_set = VK_NULL_HANDLE;
        other.m_image_available = VK_NULL_HANDLE;
        other.m_render_finished = VK_NULL_HANDLE;
        other.m_in_flight = VK_NULL_HANDLE;
    }

    VkCommandBuffer FrameContext::command_buffer() const
    {
        return m_cmd ? m_cmd->handle() : VK_NULL_HANDLE;
    }

    VkCommandBuffer FrameContext::begin(GpuSignal signal)
    {
        m_cmd->reset();
        m_cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        return m_cmd->handle();
    }

    void FrameContext::end()
    {
        m_cmd->end();
    }

} // namespace ankh
