// src/frame/frame-context.cpp
#include "frame/frame-context.hpp"

#include "commands/command-buffer.hpp"
#include "commands/command-pool.hpp"
#include "memory/buffer.hpp"

#include <stdexcept>

namespace ankh
{

    FrameContext::FrameContext(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t graphicsQueueFamilyIndex,
                               VkDeviceSize uniformBufferSize, VkDescriptorSet descriptorSet)
        : m_device(device)
        , m_descriptor_set(descriptorSet)
    {
        // Command pool for this frame
        m_pool = std::make_unique<CommandPool>(m_device, graphicsQueueFamilyIndex);

        // Command buffer for this frame
        m_cmd = std::make_unique<CommandBuffer>(m_device, m_pool->handle());

        // Per-frame uniform buffer (host visible & coherent)
        m_uniform_buffer = std::make_unique<Buffer>(physicalDevice,
                                                    m_device,
                                                    uniformBufferSize,
                                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        m_uniform_mapped = m_uniform_buffer->map(0, uniformBufferSize);

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniform_buffer->handle();
        bufferInfo.offset = 0;
        bufferInfo.range = uniformBufferSize;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = m_descriptor_set;
        write.dstBinding = 0; // binding = 0 in your shader
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);

        // Sync primitives for this frame
        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(m_device, &semInfo, nullptr, &m_image_available) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semInfo, nullptr, &m_render_finished) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create frame semaphores");
        }

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // Start signaled so first frame doesn't block
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(m_device, &fenceInfo, nullptr, &m_in_flight) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create frame fence");
        }
    }

    FrameContext::~FrameContext()
    {
        if (m_device == VK_NULL_HANDLE)
        {
            return;
        }

        if (m_uniform_buffer && m_uniform_mapped)
        {
            m_uniform_buffer->unmap();
            m_uniform_mapped = nullptr;
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

        // unique_ptrs will clean up their own Vulkan resources:
        // CommandPool, CommandBuffer, Buffer
    }

    FrameContext::FrameContext(FrameContext &&other) noexcept
        : m_device(other.m_device)
        , m_pool(std::move(other.m_pool))
        , m_cmd(std::move(other.m_cmd))
        , m_uniform_buffer(std::move(other.m_uniform_buffer))
        , m_uniform_mapped(other.m_uniform_mapped)
        , m_descriptor_set(other.m_descriptor_set)
        , m_image_available(other.m_image_available)
        , m_render_finished(other.m_render_finished)
        , m_in_flight(other.m_in_flight)
    {
        other.m_device = VK_NULL_HANDLE;
        other.m_uniform_mapped = nullptr;
        other.m_descriptor_set = VK_NULL_HANDLE;
        other.m_image_available = VK_NULL_HANDLE;
        other.m_render_finished = VK_NULL_HANDLE;
        other.m_in_flight = VK_NULL_HANDLE;
    }

    VkCommandBuffer FrameContext::command_buffer() const { return m_cmd ? m_cmd->handle() : VK_NULL_HANDLE; }

    VkCommandBuffer FrameContext::begin()
    {
        m_cmd->reset();
        m_cmd->begin();
        return m_cmd->handle();
    }

    void FrameContext::end() { m_cmd->end(); }

} // namespace ankh
