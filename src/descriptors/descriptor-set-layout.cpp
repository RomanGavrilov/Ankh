// src/descriptors/descriptor-set-layout.cpp
#include "descriptor-set-layout.hpp"

#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    DescriptorSetLayout::DescriptorSetLayout(VkDevice device)
        : m_device(device)
    {
        // Binding 0: UBO (vertex shader)
        VkDescriptorSetLayoutBinding ubo{};
        ubo.binding = 0;
        ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo.descriptorCount = 1;
        ubo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        ubo.pImmutableSamplers = nullptr;

        // Binding 1: combined image sampler (fragment shader)
        VkDescriptorSetLayoutBinding sampler{};
        sampler.binding = 1;
        sampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler.descriptorCount = 1;
        sampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        sampler.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding bindings[] = {ubo, sampler};

        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 2;
        info.pBindings = bindings;

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
