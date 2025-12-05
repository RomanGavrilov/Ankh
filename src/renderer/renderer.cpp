// src/renderer/renderer.cpp

#include "renderer/renderer.hpp"

#include "platform/window.hpp"

#include "core/context.hpp"

#include "swapchain/swapchain.hpp"

#include "renderpass/frame-buffer.hpp"
#include "renderpass/render-pass.hpp"

#include "draw-pass.hpp"
#include "scene-renderer.hpp"
#include "scene/camera.hpp"
#include "scene/material.hpp"
#include "scene/mesh.hpp"
#include "scene/renderable.hpp"

#include "ui-pass.hpp"

#include "descriptors/descriptor-pool.hpp"
#include "descriptors/descriptor-set-layout.hpp"

#include "pipeline/graphics-pipeline.hpp"
#include "pipeline/pipeline-layout.hpp"

#include "utils/config.hpp"
#include "utils/types.hpp"

#include "memory/buffer.hpp"
#include "memory/texture.hpp"
#include "memory/upload-context.hpp"

#include "commands/command-buffer.hpp"
#include "commands/command-pool.hpp"

#include "frame/frame-context.hpp"

#include "sync/frame-sync.hpp"

#include <chrono>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    Renderer::Renderer()
    {
        ANKH_LOG_INFO("[Renderer] ObjectBuffer capacity per frame: " +
                      std::to_string(ankh::config().maxObjects));

        init_vulkan();
    }

    Renderer::~Renderer()
    {
        if (m_context)
        {
            vkDeviceWaitIdle(m_context->device_handle());
        }
    }

    void Renderer::init_vulkan()
    {
        m_window = std::make_unique<Window>("Vulkan", kWidth, kHeight);

        // Context sets up instance, debug, surface, physical device, device
        m_context = std::make_unique<Context>(m_window->handle());

        // Upload context: device + graphics queue family index
        m_upload_context =
            std::make_unique<UploadContext>(m_context->device_handle(),
                                            m_context->queues().graphicsFamily.value());

        m_swapchain = std::make_unique<Swapchain>(m_context->physical_device(),
                                                  m_context->device_handle(),
                                                  m_context->surface_handle(),
                                                  m_window->handle());

        m_render_pass =
            std::make_unique<RenderPass>(m_context->device_handle(), m_swapchain->image_format());

        m_descriptor_set_layout = std::make_unique<DescriptorSetLayout>(m_context->device_handle());

        m_pipeline_layout = std::make_unique<PipelineLayout>(m_context->device_handle(),
                                                             m_descriptor_set_layout->handle());

        m_graphics_pipeline = std::make_unique<GraphicsPipeline>(m_context->device_handle(),
                                                                 m_render_pass->handle(),
                                                                 m_pipeline_layout->handle());

        m_draw_pass = std::make_unique<DrawPass>(m_context->device_handle(),
                                                 *m_swapchain,
                                                 *m_render_pass,
                                                 *m_graphics_pipeline,
                                                 *m_pipeline_layout);

        m_ui_pass = std::make_unique<UiPass>(m_context->device_handle(),
                                             *m_swapchain,
                                             *m_render_pass,
                                             *m_graphics_pipeline,
                                             *m_pipeline_layout);

        m_scene_renderer = std::make_unique<SceneRenderer>();

        m_mesh = std::make_unique<Mesh>(Mesh::make_colored_quad());

        {
            Renderable r1{};
            r1.mesh = m_mesh.get();
            r1.material = &m_scene_renderer->material();
            r1.base_transform = glm::mat4(1.0f);
            r1.base_transform = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 0.0f, 0.0f));

            Renderable r2{};
            r2.mesh = m_mesh.get();
            r2.material = &m_scene_renderer->material();
            r2.base_transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.0f, 0.0f));
            r2.transform = r2.base_transform;

            m_scene_renderer->renderables().push_back(r1);
            m_scene_renderer->renderables().push_back(r2);
        }

        create_framebuffers();
        create_vertex_buffer();
        create_index_buffer();
        create_descriptor_pool();
        create_texture();
        create_frames();

        m_frame_sync = std::make_unique<FrameSync>(kMaxFramesInFlight);
    }

    void Renderer::create_framebuffers()
    {
        m_swapchain->create_framebuffers(m_render_pass->handle());
    }

    void Renderer::cleanup_swapchain()
    {
        if (m_swapchain)
        {
            m_swapchain->destroy_framebuffers();
        }

        m_swapchain.reset();
        m_render_pass.reset();
    }

    void Renderer::create_vertex_buffer()
    {
        const auto &verts = m_mesh->vertices();
        VkDeviceSize size = sizeof(Vertex) * verts.size();

        Buffer staging(m_context->physical_device().handle(),
                       m_context->device_handle(),
                       size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            void *data = staging.map(0, size);
            std::memcpy(data, verts.data(), static_cast<size_t>(size));
            staging.unmap();
        }

        m_vertex_buffer = std::make_unique<Buffer>(m_context->physical_device().handle(),
                                                   m_context->device_handle(),
                                                   size,
                                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_upload_context->copy_buffer(m_context->graphics_queue(),
                                      staging.handle(),
                                      m_vertex_buffer->handle(),
                                      size);
    }

    void Renderer::create_index_buffer()
    {
        const auto &indices = m_mesh->indices();
        VkDeviceSize size = sizeof(uint16_t) * indices.size();

        Buffer staging(m_context->physical_device().handle(),
                       m_context->device_handle(),
                       size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            void *data = staging.map(0, size);
            std::memcpy(data, indices.data(), static_cast<size_t>(size));
            staging.unmap();
        }

        m_index_buffer = std::make_unique<Buffer>(m_context->physical_device().handle(),
                                                  m_context->device_handle(),
                                                  size,
                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_upload_context->copy_buffer(m_context->graphics_queue(),
                                      staging.handle(),
                                      m_index_buffer->handle(),
                                      size);
    }

    void Renderer::create_descriptor_pool()
    {
        m_descriptor_pool =
            std::make_unique<DescriptorPool>(m_context->device_handle(), kMaxFramesInFlight);
    }

    void Renderer::create_texture()
    {
        // Tiny 2x2 checkerboard (white/black) in RGBA8
        const uint32_t texWidth = 2;
        const uint32_t texHeight = 2;
        const VkDeviceSize imageSize = texWidth * texHeight * 4; // 4 bytes per pixel

        const std::array<uint8_t, 16> pixels = {// row 0: white, black
                                                255,
                                                255,
                                                255,
                                                255, // RGBA
                                                0,
                                                0,
                                                0,
                                                255,
                                                // row 1: black, white
                                                0,
                                                0,
                                                0,
                                                255,
                                                255,
                                                255,
                                                255,
                                                255};

        // Staging buffer
        Buffer staging(m_context->physical_device().handle(),
                       m_context->device_handle(),
                       imageSize,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            void *data = staging.map(0, imageSize);
            std::memcpy(data, pixels.data(), static_cast<size_t>(imageSize));
            staging.unmap();
        }

        // Device-local texture
        m_texture =
            std::make_unique<Texture>(m_context->physical_device().handle(),
                                      m_context->device_handle(),
                                      texWidth,
                                      texHeight,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);

        // Upload via UploadContext
        VkQueue graphicsQueue = m_context->graphics_queue();

        m_upload_context->transition_image_layout(graphicsQueue,
                                                  m_texture->image(),
                                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        m_upload_context->copy_buffer_to_image(graphicsQueue,
                                               staging.handle(),
                                               m_texture->image(),
                                               texWidth,
                                               texHeight);

        m_upload_context->transition_image_layout(graphicsQueue,
                                                  m_texture->image(),
                                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void Renderer::create_frames()
    {
        m_frames.clear();
        m_frames.reserve(kMaxFramesInFlight);

        QueueFamilyIndices queues = m_context->queues();
        uint32_t graphicsFamily = queues.graphicsFamily.value();

        VkDeviceSize uboSize = sizeof(FrameUBO);
        VkDeviceSize objectSize = sizeof(ObjectDataGPU) * ankh::config().maxObjects;

        std::vector<VkDescriptorSetLayout> layouts(kMaxFramesInFlight,
                                                   m_descriptor_set_layout->handle());

        VkDescriptorSetAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = m_descriptor_pool->handle();
        ai.descriptorSetCount = kMaxFramesInFlight;
        ai.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> sets(kMaxFramesInFlight);

        ANKH_VK_CHECK(vkAllocateDescriptorSets(m_context->device_handle(), &ai, sets.data()));

        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            m_frames.emplace_back(m_context->physical_device().handle(),
                                  m_context->device_handle(),
                                  graphicsFamily,
                                  uboSize,
                                  objectSize,
                                  sets[i],
                                  m_texture->view(),
                                  m_texture->sampler());
        }
    }

    void Renderer::record_command_buffer(FrameContext &frame, uint32_t image_index)
    {
        VkCommandBuffer cmd = frame.begin();

        // --- Begin render pass ---
        VkRenderPassBeginInfo rp_info{};
        rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_info.renderPass = m_render_pass->handle();
        rp_info.framebuffer = m_swapchain->framebuffer(image_index).handle();
        rp_info.renderArea.offset = {0, 0};
        rp_info.renderArea.extent = m_swapchain->extent();

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f}; // color
        clear_values[1].depthStencil = {1.0f, 0};         // depth (clear to far, stencil 0)

        rp_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        rp_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(cmd, &rp_info, VK_SUBPASS_CONTENTS_INLINE);

        // --- Viewport / scissor ---
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapchain->extent().width);
        viewport.height = static_cast<float>(m_swapchain->extent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swapchain->extent();
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        // --- Scene draw pass ---
        const uint32_t index_count = static_cast<uint32_t>(m_mesh->index_count());
        m_draw_pass->record(cmd,
                            frame,
                            image_index,
                            m_vertex_buffer->handle(),
                            m_index_buffer->handle(),
                            index_count,
                            *m_scene_renderer);

        // --- UI pass ---
        m_ui_pass->record(cmd,
                          frame,
                          image_index,
                          m_vertex_buffer->handle(),
                          m_index_buffer->handle(),
                          index_count);

        // --- End render pass + command buffer ---
        vkCmdEndRenderPass(cmd);
        frame.end();
    }

    void Renderer::update_uniform_buffer(FrameContext &frame)
    {
        static auto start = std::chrono::high_resolution_clock::now();
        float time =
            std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();

        // 1. Update scene (camera + renderable transforms)
        m_scene_renderer->update_frame(frame, *m_swapchain, time);

        // 2. Write FrameUBO
        FrameUBO fubo{};
        const auto &cam = m_scene_renderer->camera();
        fubo.view = cam.view();
        fubo.proj = cam.proj();
        fubo.globalAlbedo = glm::vec4(1.0f); // or some global tint

        std::memcpy(frame.uniform_mapped(), &fubo, sizeof(fubo));

        // 3. Write ObjectDataGPU array
        auto *objData = reinterpret_cast<ObjectDataGPU *>(frame.object_mapped());
        auto &renderables = m_scene_renderer->renderables();

        const uint32_t capacity = frame.object_capacity();
        const uint32_t requested = static_cast<uint32_t>(renderables.size());
        const uint32_t count = std::min<uint32_t>(requested, capacity);

        if (requested > capacity)
        {
            ANKH_LOG_WARN("Renderables exceed object buffer capacity (requested=" +
                          std::to_string(requested) + ", capacity=" + std::to_string(capacity) +
                          "); extra objects will not be drawn this frame.");
        }

        for (uint32_t i = 0; i < count; ++i)
        {
            const auto &r = renderables[i];
            if (!r.mesh || !r.material)
            {
                // Fill with identity/default if you like
                objData[i].model = glm::mat4(1.0f);
                objData[i].albedo = glm::vec4(1.0f);
                continue;
            }

            objData[i].model = r.transform;
            objData[i].albedo = r.material->albedo();
        }
    }

    void Renderer::draw_frame()
    {
        auto &frame = m_frames[m_frame_sync->current()];

        VkFence fence = frame.in_flight_fence();
        vkWaitForFences(m_context->device_handle(), 1, &fence, VK_TRUE, UINT64_MAX);

        uint32_t image_index = 0;
        VkResult result = vkAcquireNextImageKHR(m_context->device_handle(),
                                                m_swapchain->handle(),
                                                UINT64_MAX,
                                                frame.image_available(),
                                                VK_NULL_HANDLE,
                                                &image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate_swapchain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swapchain image");
        }

        update_uniform_buffer(frame);
        vkResetFences(m_context->device_handle(), 1, &fence);

        record_command_buffer(frame, image_index);

        VkCommandBuffer cmd = frame.command_buffer();

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSemaphore waitSem = frame.image_available();
        VkSemaphore signalSem = frame.render_finished();

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &waitSem;
        submit.pWaitDstStageMask = waitStages;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &signalSem;

        ANKH_VK_CHECK(vkQueueSubmit(m_context->graphics_queue(), 1, &submit, fence));

        VkPresentInfoKHR present{};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &signalSem;

        VkSwapchainKHR swapchains[] = {m_swapchain->handle()};
        present.swapchainCount = 1;
        present.pSwapchains = swapchains;
        present.pImageIndices = &image_index;

        result = vkQueuePresentKHR(m_context->present_queue(), &present);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            m_window->framebuffer_resized())
        {
            m_window->set_framebuffer_resized(false);
            recreate_swapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swapchain image");
        }

        m_frame_sync->advance();
    }

    void Renderer::recreate_swapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window->handle(), &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window->handle(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_context->device_handle());

        cleanup_swapchain();

        m_swapchain = std::make_unique<Swapchain>(m_context->physical_device(),
                                                  m_context->device_handle(),
                                                  m_context->surface_handle(),
                                                  m_window->handle());

        m_render_pass =
            std::make_unique<RenderPass>(m_context->device_handle(), m_swapchain->image_format());

        create_framebuffers();
    }

    void Renderer::run()
    {
        while (!glfwWindowShouldClose(m_window->handle()))
        {
            glfwPollEvents();
            draw_frame();
        }

        vkDeviceWaitIdle(m_context->device_handle());
    }

} // namespace ankh
