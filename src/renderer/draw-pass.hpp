// src/renderer/draw-pass.hpp
#pragma once

#include "utils/types.hpp"

namespace ankh
{
    class Swapchain;
    class RenderPass;
    class GraphicsPipeline;
    class PipelineLayout;
    class FrameContext;

    class DrawPass
    {
    public:
        DrawPass(VkDevice device,
                 const Swapchain& swapchain,
                 const RenderPass& render_pass,
                 const GraphicsPipeline& pipeline,
                 const PipelineLayout& layout);

        // Record a full frame’s draw commands into the frame’s command buffer
        void record(FrameContext& frame,
                    uint32_t image_index,
                    VkBuffer vertex_buffer,
                    VkBuffer index_buffer,
                    uint32_t index_count);

    private:
        VkDevice m_device{VK_NULL_HANDLE};
        const Swapchain& m_swapchain;
        const RenderPass& m_render_pass;
        const GraphicsPipeline& m_pipeline;
        const PipelineLayout& m_layout;
    };

} // namespace ankh
