#include "pipeline/pipeline-layout.hpp"
#include <stdexcept>

namespace ankh
{

    PipelineLayout::PipelineLayout(VkDevice device, VkDescriptorSetLayout set_layout)
        : m_device(device)
    {

        VkPipelineLayoutCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        ci.setLayoutCount = 1;
        ci.pSetLayouts = &set_layout;

        if (vkCreatePipelineLayout(m_device, &ci, nullptr, &m_layout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout");
    }

    PipelineLayout::~PipelineLayout()
    {
        if (m_layout)
            vkDestroyPipelineLayout(m_device, m_layout, nullptr);
    }

} // namespace ankh
