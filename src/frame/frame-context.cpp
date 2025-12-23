// src/frame/frame-context.cpp
#include "frame/frame-context.hpp"

#include "commands/command-buffer.hpp"
#include "commands/command-pool.hpp"
#include "descriptors/descriptor-writer.hpp"
#include "memory/buffer.hpp"
#include "utils/gpu-retirement-queue.hpp"

#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    FrameContext::FrameContext(VmaAllocator allocator,
                               VkDevice device,
                               uint32_t graphicsQueueFamilyIndex,
                               VkDeviceSize uniformBufferSize,
                               VkDeviceSize objectBufferSize,
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

        // Uniform buffer (FrameUBO)
        m_uniform_buffer = std::make_unique<Buffer>(allocator,
                                                    device,
                                                    uniformBufferSize,
                                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                    VMA_MEMORY_USAGE_CPU_TO_GPU,
                                                    retirement,
                                                    GpuSignal{});

        m_uniform_mapped = m_uniform_buffer->map();

        // Object buffer (ObjectDataGPU array)
        m_object_buffer = std::make_unique<Buffer>(allocator,
                                                   m_device,
                                                   objectBufferSize,
                                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                   VMA_MEMORY_USAGE_CPU_TO_GPU,
                                                   retirement,
                                                   GpuSignal{},
                                                   VMA_ALLOCATION_CREATE_MAPPED_BIT);

        m_object_mapped = m_object_buffer->map();

        // How many ObjectDataGPU elements fit in the object buffer?
        m_object_capacity = static_cast<uint32_t>(objectBufferSize / sizeof(ObjectDataGPU));

        // Update descriptor set for this frame
        DescriptorWriter writer{m_device};

        writer.writeUniformBuffer(m_descriptor_set,
                                  m_uniform_buffer->handle(),
                                  uniformBufferSize,
                                  /*binding*/ 0);

        writer.writeStorageBuffer(m_descriptor_set,
                                  m_object_buffer->handle(),
                                  objectBufferSize,
                                  /*binding*/ 1);

        writer.writeCombinedImageSampler(m_descriptor_set,
                                         textureView,
                                         textureSampler,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                         /*binding*/ 2);

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

        if (m_uniform_buffer && m_uniform_mapped)
        {
            m_uniform_buffer->unmap();
            m_uniform_mapped = nullptr;
        }

        if (m_object_buffer && m_object_mapped)
        {
            m_object_buffer->unmap();
            m_object_mapped = nullptr;
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

    VkCommandBuffer FrameContext::command_buffer() const
    {
        return m_cmd ? m_cmd->handle() : VK_NULL_HANDLE;
    }

    VkCommandBuffer FrameContext::begin(GpuSignal signal)
    {
        if (m_retirement)
        {
            m_uniform_buffer->set_retirement(m_retirement, signal);
            m_object_buffer->set_retirement(m_retirement, signal);
        }

        m_cmd->reset();
        m_cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        return m_cmd->handle();
    }

    void FrameContext::end()
    {
        m_cmd->end();
    }

} // namespace ankh
