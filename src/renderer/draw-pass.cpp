// src/renderer/draw-pass.cpp
#include "renderer/draw-pass.hpp"

#include "swapchain/swapchain.hpp"
#include "renderpass/render-pass.hpp"
#include "pipeline/graphics-pipeline.hpp"
#include "pipeline/pipeline-layout.hpp"
#include "frame/frame-context.hpp"
#include "utils/types.hpp"

namespace ankh
{

    DrawPass::DrawPass(VkDevice device,
                       const Swapchain& swapchain,
                       const RenderPass& render_pass,
                       const GraphicsPipeline& pipeline,
                       const PipelineLayout& layout)
        : m_device(device)
        , m_swapchain(swapchain)
        , m_render_pass(render_pass)
        , m_pipeline(pipeline)
        , m_layout(layout)
    {}

    void DrawPass::record(FrameContext& frame,
                          uint32_t image_index,
                          VkBuffer vertex_buffer,
                          VkBuffer index_buffer,
                          uint32_t index_count)
    {
        // Let FrameContext reset & begin its command buffer
        VkCommandBuffer cmd = frame.begin();

        // --- Begin render pass ---
        VkRenderPassBeginInfo rp_info{};
        rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_info.renderPass = m_render_pass.handle();
        rp_info.framebuffer = m_swapchain.framebuffer(image_index).handle();
        rp_info.renderArea.offset = {0, 0};
        rp_info.renderArea.extent = m_swapchain.extent();

        VkClearValue clear_value{};
        clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
        rp_info.clearValueCount = 1;
        rp_info.pClearValues = &clear_value;

        vkCmdBeginRenderPass(cmd, &rp_info, VK_SUBPASS_CONTENTS_INLINE);

        // --- Viewport / scissor ---
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width  = static_cast<float>(m_swapchain.extent().width);
        viewport.height = static_cast<float>(m_swapchain.extent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swapchain.extent();
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        // --- Bind pipeline + resources ---
        vkCmdBindPipeline(cmd,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipeline.handle());

        VkBuffer vertex_buffers[] = { vertex_buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(cmd, index_buffer, 0, VK_INDEX_TYPE_UINT16);

        VkDescriptorSet ds = frame.descriptor_set();
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_layout.handle(),
            0, 1, &ds,
            0, nullptr
        );

        // --- Draw ---
        vkCmdDrawIndexed(cmd, index_count, 1, 0, 0, 0);

        // --- End render pass + command buffer ---
        vkCmdEndRenderPass(cmd);
        frame.end();
    }

} // namespace ankh
