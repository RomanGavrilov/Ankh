// src/renderer/ui-pass.hpp
#pragma once

#include "utils/types.hpp"

namespace ankh
{
    class Swapchain;
    class RenderPass;
    class GraphicsPipeline;
    class PipelineLayout;
    class FrameContext;

    // Simple second pass (e.g. UI/overlay). For now it just issues another draw.
    class UiPass
    {
      public:
        UiPass(VkDevice device,
               const Swapchain &swapchain,
               const RenderPass &render_pass,
               const GraphicsPipeline &pipeline,
               const PipelineLayout &layout);

        // Assumes:
        //  - command buffer already begun
        //  - render pass already active
        //  - viewport/scissor already set
        void record(VkCommandBuffer cmd,
                    FrameContext &frame,
                    uint32_t image_index,
                    VkBuffer vertex_buffer,
                    VkBuffer index_buffer,
                    uint32_t index_count);

      private:
        VkDevice m_device{VK_NULL_HANDLE};
        const Swapchain &m_swapchain;
        const RenderPass &m_render_pass;
        const GraphicsPipeline &m_pipeline;
        const PipelineLayout &m_layout;
    };

} // namespace ankh
