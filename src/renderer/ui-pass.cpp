// src/renderer/ui-pass.cpp
#include "renderer/ui-pass.hpp"

#include "frame/frame-context.hpp"
#include "pipeline/graphics-pipeline.hpp"
#include "pipeline/pipeline-layout.hpp"
#include "renderpass/render-pass.hpp"
#include "swapchain/swapchain.hpp"
#include "utils/types.hpp"

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

    void UiPass::record(VkCommandBuffer cmd,
                        FrameContext &frame,
                        uint32_t /*image_index*/,
                        VkBuffer vertex_buffer,
                        VkBuffer index_buffer,
                        uint32_t index_count)
    {
        // For now, reuse the same pipeline & descriptor set as the main draw.
        // Later this can be a dedicated UI pipeline with its own shaders/layout.

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.handle());

        VkBuffer vertexBuffers[] = {vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(cmd, index_buffer, 0, VK_INDEX_TYPE_UINT16);

        VkDescriptorSet ds = frame.descriptor_set();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout.handle(), 0, 1, &ds, 0, nullptr);

        // A second draw – conceptually your "UI" or overlay.
        vkCmdDrawIndexed(cmd, index_count, 1, 0, 0, 0);
    }

} // namespace ankh
