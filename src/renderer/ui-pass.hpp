// src/renderer/ui-pass.hpp
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

    // Simple second pass (UI/overlay hook). Currently no geometry.
    class UiPass
    {
      public:
        UiPass(VkDevice device,
               const Swapchain &swapchain,
               const RenderPass &render_pass,
               const GraphicsPipeline &pipeline,
               const PipelineLayout &layout);

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
