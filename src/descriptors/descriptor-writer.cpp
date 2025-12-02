// src/descriptors/descriptor-writer.cpp
#include "descriptor-writer.hpp"

namespace ankh
{

    DescriptorWriter::DescriptorWriter(VkDevice device)
        : m_device(device)
    {
    }

    void DescriptorWriter::writeUniformBuffer(VkDescriptorSet set, VkBuffer buf, VkDeviceSize size, uint32_t binding)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buf;
        bufferInfo.offset = 0;
        bufferInfo.range = size;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
    }

    void DescriptorWriter::writeCombinedImageSampler(VkDescriptorSet set,
                                                     VkImageView view,
                                                     VkSampler sampler,
                                                     VkImageLayout layout,
                                                     uint32_t binding)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = sampler;
        imageInfo.imageView = view;
        imageInfo.imageLayout = layout;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
    }

} // namespace ankh
