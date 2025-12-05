#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class DescriptorWriter
    {
      public:
        explicit DescriptorWriter(VkDevice device);

        // 0: uniform (FrameUBO)
        void writeUniformBuffer(VkDescriptorSet set,
                                VkBuffer buf,
                                VkDeviceSize size,
                                uint32_t binding = 0);

        // 1: storage (ObjectBuffer)
        void writeStorageBuffer(VkDescriptorSet set,
                                VkBuffer buf,
                                VkDeviceSize size,
                                uint32_t binding = 1);

        // 2: combined image sampler
        void
        writeCombinedImageSampler(VkDescriptorSet set,
                                  VkImageView view,
                                  VkSampler sampler,
                                  VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  uint32_t binding = 2);

      private:
        VkDevice m_device{};
    };

} // namespace ankh
