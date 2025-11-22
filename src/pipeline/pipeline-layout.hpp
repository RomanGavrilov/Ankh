#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class PipelineLayout
    {
    public:
        PipelineLayout(VkDevice device, VkDescriptorSetLayout set_layout);
        ~PipelineLayout();

        VkPipelineLayout handle() const { return m_layout; }

    private:
        VkDevice m_device{};
        VkPipelineLayout m_layout{};
    };

} // namespace ankh
