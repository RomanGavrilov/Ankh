#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class DescriptorWriter
    {
      public:
        explicit DescriptorWriter(VkDevice device);

        // Binding 0: uniform buffer
        void writeUniformBuffer(VkDescriptorSet set, VkBuffer buf, VkDeviceSize size, uint32_t binding = 0);

        // Binding 1: combined image sampler
        void writeCombinedImageSampler(VkDescriptorSet set,
                                       VkImageView view,
                                       VkSampler sampler,
                                       VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                       uint32_t binding = 1);

      private:
        VkDevice m_device{};
    };

} // namespace ankh
