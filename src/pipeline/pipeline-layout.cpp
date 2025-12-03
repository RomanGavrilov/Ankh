#include "pipeline/pipeline-layout.hpp"
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    PipelineLayout::PipelineLayout(VkDevice device, VkDescriptorSetLayout set_layout)
        : m_device(device)
    {

        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount = 1;
        info.pSetLayouts = &set_layout;

        ANKH_VK_CHECK(vkCreatePipelineLayout(m_device, &info, nullptr, &m_layout));
    }

    PipelineLayout::~PipelineLayout()
    {
        if (m_layout)
            vkDestroyPipelineLayout(m_device, m_layout, nullptr);
    }

} // namespace ankh
