// src/renderer/draw-pass.cpp
#include "renderer/draw-pass.hpp"

#include "frame/frame-context.hpp"
#include "pipeline/graphics-pipeline.hpp"
#include "pipeline/pipeline-layout.hpp"
#include "renderpass/render-pass.hpp"
#include "swapchain/swapchain.hpp"
#include "utils/types.hpp"

namespace ankh
{

    DrawPass::DrawPass(VkDevice device,
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

    void DrawPass::record(VkCommandBuffer cmd,
                          FrameContext &frame,
                          uint32_t /*image_index*/,
                          VkBuffer vertex_buffer,
                          VkBuffer index_buffer,
                          uint32_t index_count)
    {

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.handle());

        // Bind vertex buffer
        VkBuffer vertexBuffers[] = {vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        // Bind index buffer
        vkCmdBindIndexBuffer(cmd, index_buffer, 0, VK_INDEX_TYPE_UINT16);

        // Bind descriptor set for this frame
        VkDescriptorSet ds = frame.descriptor_set();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout.handle(), 0, 1, &ds, 0, nullptr);

        // Draw indexed
        vkCmdDrawIndexed(cmd, index_count, 1, 0, 0, 0);
    }

} // namespace ankh
