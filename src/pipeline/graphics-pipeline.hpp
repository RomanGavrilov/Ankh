#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(VkDevice device,
                         VkRenderPass render_pass,
                         VkPipelineLayout layout);

        ~GraphicsPipeline();

        VkPipeline handle() const { return m_pipeline; }

    private:
        VkDevice m_device{};
        VkPipeline m_pipeline{};
    };

} // namespace ankh
