#include "descriptors/descriptor-set-layout.hpp"
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    DescriptorSetLayout::DescriptorSetLayout(VkDevice device)
        : m_device(device)
    {

        VkDescriptorSetLayoutBinding ubo{};
        ubo.binding = 0;
        ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo.descriptorCount = 1;
        ubo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        ubo.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ci.bindingCount = 1;
        ci.pBindings = &ubo;

        ANKH_VK_CHECK(vkCreateDescriptorSetLayout(m_device, &ci, nullptr, &m_layout));
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        if (m_layout)
        {
            vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
        }
    }

} // namespace ankh
