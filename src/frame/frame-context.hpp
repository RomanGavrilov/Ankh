// src/frame/frame-context.hpp
#pragma once

#include "utils/types.hpp"
#include <memory>
#include <vk_mem_alloc.h>

namespace ankh
{

    class CommandPool;
    class CommandBuffer;
    class Buffer;
    class GpuRetirementQueue;
    class GpuSignal;

    class FrameContext
    {
      public:
        FrameContext() = default;

        FrameContext(VmaAllocator allocator,
                     VkDevice device,
                     uint32_t graphicsQueueFamilyIndex,
                     VkDeviceSize objectBufferSize,
                     VkDescriptorSet descriptorSet,
                     VkImageView textureView,
                     VkSampler textureSampler,
                     GpuRetirementQueue *retirement);

        ~FrameContext();

        FrameContext(const FrameContext &) = delete;

        FrameContext &operator=(const FrameContext &) = delete;

        FrameContext(FrameContext &&) noexcept;

        // Accessors
        VkCommandBuffer command_buffer() const;

        void *object_mapped() const
        {
            return m_object_mapped;
        }

        VkDescriptorSet descriptor_set() const
        {
            return m_descriptor_set;
        }

        VkSemaphore image_available() const
        {
            return m_image_available;
        }

        VkSemaphore render_finished() const
        {
            return m_render_finished;
        }

        VkFence in_flight_fence() const
        {
            return m_in_flight;
        }

        uint32_t object_capacity() const
        {
            return m_object_capacity;
        }

        // Frame lifecycle
        VkCommandBuffer begin(GpuSignal signal);
        void end();

      private:
        VkDevice m_device{VK_NULL_HANDLE};

        std::unique_ptr<CommandPool> m_pool;
        std::unique_ptr<CommandBuffer> m_cmd;

        std::unique_ptr<Buffer> m_object_buffer;
        void *m_object_mapped{};

        VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};

        VkSemaphore m_image_available{VK_NULL_HANDLE};
        VkSemaphore m_render_finished{VK_NULL_HANDLE};
        VkFence m_in_flight{VK_NULL_HANDLE};

        uint32_t m_object_capacity{0};

        GpuRetirementQueue *m_retirement{nullptr};
    };

} // namespace ankh
