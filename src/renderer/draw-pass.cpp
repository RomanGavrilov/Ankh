// src/renderer/draw-pass.cpp
#include "renderer/draw-pass.hpp"

#include <algorithm>

#include "frame/frame-context.hpp"
#include "pipeline/graphics-pipeline.hpp"
#include "pipeline/pipeline-layout.hpp"
#include "renderer/scene-renderer.hpp"
#include "renderpass/render-pass.hpp"
#include "swapchain/swapchain.hpp"

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
                          const std::unordered_map<MeshHandle, MeshDrawInfo> &mesh_draw_info,
                          SceneRenderer &scene_renderer)
    {
        if (vertex_buffer == VK_NULL_HANDLE || index_buffer == VK_NULL_HANDLE)
        {
            return;
        }

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.handle());

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, &vertex_buffer, offsets);
        vkCmdBindIndexBuffer(cmd, index_buffer, 0, VK_INDEX_TYPE_UINT16);

        VkDescriptorSet set = frame.descriptor_set();
        vkCmdBindDescriptorSets(cmd,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_layout.handle(),
                                0,
                                1,
                                &set,
                                0,
                                nullptr);

        const auto &renderables = scene_renderer.renderables();
        const uint32_t capacity = frame.object_capacity();
        const uint32_t count =
            std::min<uint32_t>(static_cast<uint32_t>(renderables.size()), capacity);

        for (uint32_t i = 0; i < count; ++i)
        {
            MeshHandle meshHandle = renderables[i].mesh;

            auto it = mesh_draw_info.find(meshHandle);

            if (it == mesh_draw_info.end())
            {
                // Mesh has no GPU range; skip (should not happen in normal cases)
                continue;
            }

            const MeshDrawInfo &info = it->second;

            struct ObjectPC
            {
                uint32_t objectIndex;
            } pc{i};

            vkCmdPushConstants(cmd,
                               m_layout.handle(),
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(ObjectPC),
                               &pc);

            vkCmdDrawIndexed(cmd, info.indexCount, 1, info.firstIndex, info.vertexOffset, 0);
        }
    }

} // namespace ankh
