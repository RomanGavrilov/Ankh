// src/frame/frame-context.hpp
#pragma once

#include "utils/types.hpp"
#include <memory>

namespace ankh
{

    class CommandPool;
    class CommandBuffer;
    class Buffer;

    class FrameContext
    {
      public:
        FrameContext() = default;

        FrameContext(VkPhysicalDevice physicalDevice,
                     VkDevice device,
                     uint32_t graphicsQueueFamilyIndex,
                     VkDeviceSize uniformBufferSize,
                     VkDescriptorSet descriptorSet,
                     VkImageView textureView,
                     VkSampler textureSampler);

        ~FrameContext();

        FrameContext(const FrameContext &) = delete;
        FrameContext &operator=(const FrameContext &) = delete;

        FrameContext(FrameContext &&) noexcept;

        // Accessors
        VkCommandBuffer command_buffer() const;
        void *uniform_mapped() const { return m_uniform_mapped; }
        VkDescriptorSet descriptor_set() const { return m_descriptor_set; }

        VkSemaphore image_available() const { return m_image_available; }
        VkSemaphore render_finished() const { return m_render_finished; }
        VkFence in_flight_fence() const { return m_in_flight; }

        // manage command buffer lifecycle for this frame
        VkCommandBuffer begin(); // reset + vkBeginCommandBuffer
        void end();              // vkEndCommandBuffer

      private:
        VkDevice m_device{VK_NULL_HANDLE};

        std::unique_ptr<CommandPool> m_pool;
        std::unique_ptr<CommandBuffer> m_cmd;
        std::unique_ptr<Buffer> m_uniform_buffer;
        void *m_uniform_mapped{};

        VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};

        VkSemaphore m_image_available{VK_NULL_HANDLE};
        VkSemaphore m_render_finished{VK_NULL_HANDLE};
        VkFence m_in_flight{VK_NULL_HANDLE};
    };

} // namespace ankh
