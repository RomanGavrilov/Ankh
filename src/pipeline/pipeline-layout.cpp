#include "pipeline/pipeline-layout.hpp"
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    PipelineLayout::PipelineLayout(VkDevice device, VkDescriptorSetLayout set_layout)
        : m_device(device)
    {
        VkPushConstantRange pcRange{};
        pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pcRange.offset = 0;
        pcRange.size = sizeof(uint32_t); // objectIndex

        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount = 1;
        info.pSetLayouts = &set_layout;
        info.pushConstantRangeCount = 1;
        info.pPushConstantRanges = &pcRange;

        ANKH_VK_CHECK(vkCreatePipelineLayout(m_device, &info, nullptr, &m_layout));
    }

    PipelineLayout::~PipelineLayout()
    {
        if (m_layout)
        {
            vkDestroyPipelineLayout(m_device, m_layout, nullptr);
        }
    }

} // namespace ankh
