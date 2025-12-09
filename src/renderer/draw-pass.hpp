// src/renderer/draw-pass.hpp
#pragma once

#include <unordered_map>

#include "renderer/mesh-draw-info.hpp"
#include "scene/renderable.hpp"
#include "utils/types.hpp"

namespace ankh
{
    class Swapchain;
    class RenderPass;
    class GraphicsPipeline;
    class PipelineLayout;
    class FrameContext;
    class SceneRenderer;

    class DrawPass
    {
      public:
        DrawPass(VkDevice device,
                 const Swapchain &swapchain,
                 const RenderPass &render_pass,
                 const GraphicsPipeline &pipeline,
                 const PipelineLayout &layout);

        // Assumes:
        //  - command buffer is already begun
        //  - render pass is already active
        //  - viewport/scissor already set
        void record(VkCommandBuffer cmd,
                    FrameContext &frame,
                    uint32_t image_index,
                    VkBuffer vertex_buffer,
                    VkBuffer index_buffer,
                    const std::unordered_map<MeshHandle, MeshDrawInfo> &mesh_draw_info,
                    SceneRenderer &scene_renderer);

      private:
        VkDevice m_device{VK_NULL_HANDLE};
        const Swapchain &m_swapchain;
        const RenderPass &m_render_pass;
        const GraphicsPipeline &m_pipeline;
        const PipelineLayout &m_layout;
    };

} // namespace ankh
