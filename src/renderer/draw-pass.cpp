// src/renderer/draw-pass.cpp
#include "renderer/draw-pass.hpp"

#include "frame/frame-context.hpp"
#include "pipeline/graphics-pipeline.hpp"
#include "pipeline/pipeline-layout.hpp"
#include "renderer/scene-renderer.hpp"
#include "renderpass/render-pass.hpp"
#include "scene/camera.hpp"
#include "scene/material.hpp"
#include "scene/renderable.hpp"
#include "swapchain/swapchain.hpp"
#include "utils/logging.hpp"
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
                          uint32_t index_count,
                          SceneRenderer &scene)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.handle());

        // Vertex buffer
        VkBuffer vertexBuffers[] = {vertex_buffer};

        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        // Index buffer
        vkCmdBindIndexBuffer(cmd, index_buffer, 0, VK_INDEX_TYPE_UINT16);

        // Descriptor set
        VkDescriptorSet ds = frame.descriptor_set();

        vkCmdBindDescriptorSets(cmd,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_layout.handle(),
                                0,
                                1,
                                &ds,
                                0,
                                nullptr);

        const auto &renderables = scene.renderables();
        const uint32_t drawCount = static_cast<uint32_t>(renderables.size());

        for (uint32_t i = 0; i < drawCount; ++i)
        {
            const auto &r = renderables[i];
            if (!r.mesh || !r.material)
            {
                ANKH_LOG_WARN("[DrawPass] Skipping renderable: missing mesh or material.");
                continue;
            }

            uint32_t objectIndex = i;

            vkCmdPushConstants(cmd,
                               m_layout.handle(),
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(uint32_t),
                               &objectIndex);

            vkCmdDrawIndexed(cmd, index_count, 1, 0, 0, 0);
        }
    }

} // namespace ankh
