// src/descriptors/descriptor-set-layout.cpp
#include "descriptor-set-layout.hpp"

#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    DescriptorSetLayout::DescriptorSetLayout(VkDevice device)
        : m_device(device)
    {
        // Binding 0: FrameUBO (uniform buffer)
        VkDescriptorSetLayoutBinding frameUBO{};
        frameUBO.binding = 0;
        frameUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        frameUBO.descriptorCount = 1;
        frameUBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        frameUBO.pImmutableSamplers = nullptr;

        // Binding 1: Object buffer (storage buffer)
        VkDescriptorSetLayoutBinding objectBuffer{};
        objectBuffer.binding = 1;
        objectBuffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        objectBuffer.descriptorCount = 1;
        objectBuffer.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        objectBuffer.pImmutableSamplers = nullptr;

        // Binding 2: combined image sampler
        VkDescriptorSetLayoutBinding sampler{};
        sampler.binding = 2;
        sampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler.descriptorCount = 1;
        sampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        sampler.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 3> bindings = {frameUBO, objectBuffer, sampler};

        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = static_cast<uint32_t>(bindings.size());
        info.pBindings = bindings.data();

        ANKH_VK_CHECK(vkCreateDescriptorSetLayout(m_device, &info, nullptr, &m_layout));
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        if (m_layout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
            m_layout = VK_NULL_HANDLE;
        }
    }

} // namespace ankh
