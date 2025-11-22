#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class DescriptorWriter
    {
    public:
        explicit DescriptorWriter(VkDevice device);
        void writeUniformBuffer(VkDescriptorSet set, VkBuffer buf, VkDeviceSize size);

    private:
        VkDevice m_device{};
    };

} // namespace ankh
