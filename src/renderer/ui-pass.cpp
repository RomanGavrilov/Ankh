// src/renderer/ui-pass.cpp
#include "renderer/ui-pass.hpp"

#include "frame/frame-context.hpp"
#include "pipeline/graphics-pipeline.hpp"
#include "pipeline/pipeline-layout.hpp"
#include "renderer/scene-renderer.hpp"
#include "renderpass/render-pass.hpp"
#include "swapchain/swapchain.hpp"

namespace ankh
{

    UiPass::UiPass(VkDevice device,
                   const Swapchain &swapchain,
                   const RenderPass &render_pass,
                   const GraphicsPipeline &pipeline,
                   const PipelineLayout &layout)
        : m_device(device)
        , m_swapchain(swapchain)
        , m_render_pass(render_pass)
        , m_pipeline(pipeline)
        , m_layout(layout)
    {
    }

    void UiPass::record(VkCommandBuffer /*cmd*/,
                        FrameContext & /*frame*/,
                        uint32_t /*image_index*/,
                        VkBuffer /*vertex_buffer*/,
                        VkBuffer /*index_buffer*/,
                        const std::unordered_map<MeshHandle, MeshDrawInfo> & /*mesh_draw_info*/,
                        SceneRenderer & /*scene_renderer*/)
    {
        // Currently a no-op. Hook for future UI/overlay rendering using the same buffers.
    }

} // namespace ankh
