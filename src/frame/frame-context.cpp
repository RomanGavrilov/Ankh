// src/frame/frame-context.cpp
#include "frame/frame-context.hpp"

#include "commands/command-buffer.hpp"
#include "commands/command-pool.hpp"
#include "memory/buffer.hpp"

#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    FrameContext::FrameContext(VkPhysicalDevice physicalDevice,
                               VkDevice device,
                               uint32_t graphicsQueueFamilyIndex,
                               VkDeviceSize uniformBufferSize,
                               VkDescriptorSet descriptorSet,
                               VkImageView textureView,
                               VkSampler textureSampler)
        : m_device(device)
        , m_descriptor_set(descriptorSet)
    {
        // Command pool & buffer
        m_pool = std::make_unique<CommandPool>(m_device, graphicsQueueFamilyIndex);

        m_cmd = std::make_unique<CommandBuffer>(m_device, m_pool->handle());

        // Per-frame UBO
        m_uniform_buffer = std::make_unique<Buffer>(physicalDevice,
                                                    m_device,
                                                    uniformBufferSize,
                                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        m_uniform_mapped = m_uniform_buffer->map(0, uniformBufferSize);

        // 🔹 Descriptor writes: binding 0 = UBO, binding 1 = texture
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniform_buffer->handle();
        bufferInfo.offset = 0;
        bufferInfo.range = uniformBufferSize;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = textureSampler;
        imageInfo.imageView = textureView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet writes[2]{};

        // Binding 0: UBO
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = m_descriptor_set;
        writes[0].dstBinding = 0;
        writes[0].dstArrayElement = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].descriptorCount = 1;
        writes[0].pBufferInfo = &bufferInfo;

        // Binding 1: combined image sampler
        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = m_descriptor_set;
        writes[1].dstBinding = 1;
        writes[1].dstArrayElement = 0;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[1].descriptorCount = 1;
        writes[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_device, 2, writes, 0, nullptr);

        // Sync primitives...
        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(m_device, &semInfo, nullptr, &m_image_available) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semInfo, nullptr, &m_render_finished) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create frame semaphores");
        }

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
