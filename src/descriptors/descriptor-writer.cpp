// src/descriptors/descriptor-writer.cpp
#include "descriptor-writer.hpp"

namespace ankh
{
    void DescriptorWriter::writeUniformBuffer(VkDescriptorSet set, VkBuffer buf, VkDeviceSize size)
    {
        VkDescriptorBufferInfo info{buf, 0, size};
        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstSet = set;
        w.dstBinding = 0;
        w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        w.descriptorCount = 1;
        w.pBufferInfo = &info;
        vkUpdateDescriptorSets(m_device, 1, &w, 0, nullptr);
    }

    DescriptorWriter::DescriptorWriter(VkDevice device) : m_device(device) {}

} // namespace ankh