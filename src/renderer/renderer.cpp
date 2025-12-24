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
#include "scene/material-pool.hpp"
#include "scene/mesh-pool.hpp"
#include "scene/model-loader.hpp"
#include "scene/model.hpp"
#include "scene/renderable.hpp"

#include "ui-pass.hpp"

#include "descriptors/descriptor-pool.hpp"
#include "descriptors/descriptor-set-layout.hpp"

#include "pipeline/graphics-pipeline.hpp"
#include "pipeline/pipeline-layout.hpp"

#include "utils/gpu-retirement-queue.hpp"
#include "utils/gpu-tracking.hpp"
#include "utils/types.hpp"

#include "memory/buffer.hpp"
#include "memory/texture.hpp"
#include "memory/upload-context.hpp"

#include "commands/command-buffer.hpp"
#include "commands/command-pool.hpp"

#include "frame/frame-context.hpp"

#include "sync/frame-ring.hpp"
#include "sync/gpu-serial.hpp"

#include "renderer/gpu-mesh-pool.hpp"
#include "renderer/mesh-draw-info.hpp"

#include "streaming/async-uploader.hpp"

#include <chrono>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    Renderer::Renderer()
    {
        ANKH_LOG_DEBUG("[Renderer] ObjectBuffer capacity per frame: " +
                       std::to_string(ankh::config().maxObjects));

        m_gpu = std::make_unique<RendererGpuState>();
        init_vulkan();
    }

    Renderer::~Renderer()
    {
        if (!m_context)
        {
            return;
        }

        if (m_gpu && m_gpu->swapchain)
        {
            retire_swapchain_resources();
        }

        wait_for_all_frames();

        vkQueueWaitIdle(m_context->present_queue());
        vkQueueWaitIdle(m_context->graphics_queue());
        vkQueueWaitIdle(m_context->transfer_queue());

        m_gpu.reset();

        if (m_retirement_queue)
        {
            m_retirement_queue->collect(UINT64_MAX, UINT64_MAX);
            m_retirement_queue->flush_all();
        }

#ifndef NDEBUG
        m_context->gpu_tracker().report_leaks();
#endif
    }

    void Renderer::init_vulkan()
    {
        m_window = std::make_unique<Window>("Ankh", ankh::config().Width, ankh::config().Height);

        // Context sets up instance, debug, surface, physical device, device
        m_context = std::make_unique<Context>(m_window->handle());

        m_gpu->swapchain = std::make_unique<Swapchain>(m_context->physical_device(),
                                                       m_context->device_handle(),
                                                       m_context->allocator().handle(),
                                                       m_context->surface_handle(),
                                                       m_window->handle(),
                                                       tracker());

        m_gpu->render_pass = std::make_unique<RenderPass>(m_context->device_handle(),
                                                          m_gpu->swapchain->image_format());

        m_gpu->descriptor_set_layout =
            std::make_unique<DescriptorSetLayout>(m_context->device_handle());

        m_gpu->pipeline_layout =
            std::make_unique<PipelineLayout>(m_context->device_handle(),
                                             m_gpu->descriptor_set_layout->handle());

        m_gpu->graphics_pipeline =
            std::make_unique<GraphicsPipeline>(m_context->device_handle(),
                                               m_gpu->render_pass->handle(),
                                               m_gpu->pipeline_layout->handle());

        m_gpu->draw_pass = std::make_unique<DrawPass>(m_context->device_handle(),
                                                      *m_gpu->swapchain,
                                                      *m_gpu->render_pass,
                                                      *m_gpu->graphics_pipeline,
                                                      *m_gpu->pipeline_layout);

        m_gpu->ui_pass = std::make_unique<UiPass>(m_context->device_handle(),
                                                  *m_gpu->swapchain,
                                                  *m_gpu->render_pass,
                                                  *m_gpu->graphics_pipeline,
                                                  *m_gpu->pipeline_layout);

        m_gpu->scene_renderer = std::make_unique<SceneRenderer>();

        const auto framesInFlight{ankh::config().framesInFlight};

        m_gpu->frame_ring = std::make_unique<FrameRing>(framesInFlight);
        m_gpu->gpu_serial = std::make_unique<GpuSerial>(framesInFlight);
        m_retirement_queue = std::make_unique<GpuRetirementQueue>();

        // Upload context: device + graphics queue family index
        m_gpu->async_uploader =
            std::make_unique<AsyncUploader>(m_context->device_handle(),
                                            m_context->queues().transferFamily.value(),
                                            m_context->transfer_queue());

        m_gpu->gpu_mesh_pool = std::make_unique<GpuMeshPool>(m_context->allocator().handle(),
                                                             m_context->device_handle(),
                                                             *m_gpu->async_uploader);

        // Load a model into the scene
        {
            // adjust path to where you actually put Box.gltf or Box.glb
            Model model =
                ModelLoader::load_gltf("D:\\Rep\\Ankh\\assets\\models\\cerberus\\cerberus.gltf",
                                       m_gpu->scene_renderer->mesh_pool(),
                                       m_gpu->scene_renderer->material_pool());

            MaterialHandle default_mat = m_gpu->scene_renderer->default_material_handle();

            for (const auto &node : model.nodes())
            {
                if (node.mesh == INVALID_MESH_HANDLE)
                {
                    continue;
                }

                Renderable r{};
                r.mesh = node.mesh;
                r.material =
                    (node.material == INVALID_MATERIAL_HANDLE) ? default_mat : node.material;
                r.base_transform = node.local_transform;
                r.transform = r.base_transform;

                m_gpu->scene_renderer->renderables().push_back(r);
            }

            SceneBounds bounds = m_gpu->scene_renderer->compute_scene_bounds();
            if (bounds.valid)
            {
                glm::vec3 center = 0.5f * (bounds.min + bounds.max);
                float radius = glm::length(bounds.max - bounds.min) * 0.5f;

                // simple heuristic distance; works for most scenes
                float distance = (radius > 0.0f) ? radius * 2.5f : 5.0f;

                auto &cam = m_gpu->scene_renderer->camera();
                cam.set_target(center);
                cam.set_position(center + glm::vec3(distance, distance, distance));
            }
        }

        m_gpu->gpu_mesh_pool->build_from_mesh_pool(m_gpu->scene_renderer->mesh_pool());

        create_framebuffers();
        create_descriptor_pool();
        create_texture();
        create_frames();
    }

    void Renderer::create_framebuffers()
    {
        m_gpu->swapchain->create_framebuffers(m_gpu->render_pass->handle());
    }

    void Renderer::cleanup_swapchain()
    {
        // These may have been moved into the deletion queue already.
        m_gpu->ui_pass.reset();
        m_gpu->draw_pass.reset();
        m_gpu->graphics_pipeline.reset();
        m_gpu->pipeline_layout.reset();
        m_gpu->render_pass.reset();
        m_gpu->swapchain.reset();
    }

    void Renderer::wait_for_all_frames()
    {
        VkDevice dev = m_context->device_handle();

        m_gpu->frame_ring->for_each_slot(
            [this, dev](FrameSlot slot)
            {
                auto &frame = m_gpu->frames[slot];
                VkFence fence = frame.in_flight_fence();
                ANKH_VK_CHECK(vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX));
            });
    }

    void Renderer::retire_swapchain_resources()
    {
        if (!m_gpu->swapchain)
        {
            return;
        }

        VkDevice device = m_context->device_handle();

        // threshold = last issued serial + framesInFlight
        const uint64_t fif = static_cast<uint64_t>(ankh::config().framesInFlight);
        const uint64_t retire_at = m_gpu->gpu_serial->last_issued() + fif;

        // 1) retire swapchain-owned resources
        auto retired = m_gpu->swapchain->retire_resources();

        m_retirement_queue->retire_after(GpuSignal::frame(retire_at),
                                         [device,
                                          views = std::move(retired.imageViews),
                                          fbs = std::move(retired.framebuffers),
                                          depth = std::move(retired.depthImage)]() mutable
                                         {
                                             for (VkImageView v : views)
                                             {
                                                 if (v)
                                                 {
                                                     vkDestroyImageView(device, v, nullptr);
                                                 }
                                             }
                                         });

        if (m_gpu->ui_pass)
        {
            m_retirement_queue->retire_after(GpuSignal::frame(retire_at),
                                             [p = std::move(m_gpu->ui_pass)]() mutable {});
        }

        if (m_gpu->draw_pass)
        {
            m_retirement_queue->retire_after(GpuSignal::frame(retire_at),
                                             [p = std::move(m_gpu->draw_pass)]() mutable {});
        }

        if (m_gpu->graphics_pipeline)
        {
            m_retirement_queue->retire_after(
                GpuSignal::frame(retire_at),
                [p = std::move(m_gpu->graphics_pipeline)]() mutable {});
        }

        if (m_gpu->pipeline_layout)
        {
            m_retirement_queue->retire_after(GpuSignal::frame(retire_at),
                                             [p = std::move(m_gpu->pipeline_layout)]() mutable {});
        }

        if (m_gpu->render_pass)
        {
            m_retirement_queue->retire_after(GpuSignal::frame(retire_at),
                                             [p = std::move(m_gpu->render_pass)]() mutable {});
        }
    }

    void Renderer::create_descriptor_pool()
    {
        m_gpu->descriptor_pool = std::make_unique<DescriptorPool>(m_context->device_handle(),
                                                                  ankh::config().framesInFlight);
    }

    void Renderer::create_texture()
    {
        // Try to find a material in the scene that has a baseColor image
        std::shared_ptr<const CpuImage> sourceImage;

        auto &renderables = m_gpu->scene_renderer->renderables();
        auto &materials = m_gpu->scene_renderer->material_pool();

        for (const auto &r : renderables)
        {
            if (!materials.valid(r.material))
            {
                continue;
            }

            const Material &mat = materials.get(r.material);
            if (mat.has_base_color_image())
            {
                sourceImage = mat.base_color_image();
                break;
            }
        }

        std::vector<uint8_t> pixels;
        uint32_t texWidth = 0;
        uint32_t texHeight = 0;

        if (sourceImage && sourceImage->width > 0 && sourceImage->height > 0)
        {
            // Convert to RGBA8 if needed
            texWidth = static_cast<uint32_t>(sourceImage->width);
            texHeight = static_cast<uint32_t>(sourceImage->height);

            const int comp = sourceImage->components;
            const auto &src = sourceImage->pixels;

            if (comp == 4)
            {
                pixels = src; // already RGBA8
            }
            else if (comp == 3)
            {
                pixels.resize(static_cast<size_t>(texWidth) * texHeight * 4);
                for (uint32_t i = 0; i < texWidth * texHeight; ++i)
                {
                    pixels[4 * i + 0] = src[3 * i + 0];
                    pixels[4 * i + 1] = src[3 * i + 1];
                    pixels[4 * i + 2] = src[3 * i + 2];
                    pixels[4 * i + 3] = 255;
                }
            }
            else
            {
                ANKH_LOG_WARN("[Renderer] Unsupported image component count in baseColorTexture; "
                              "falling back to checkerboard");
            }
        }

        // Fallback if no suitable glTF image
        if (pixels.empty())
        {
            ANKH_LOG_WARN(
                "[Renderer] No suitable baseColorTexture found; using checkerboard fallback");

            // Tiny 2x2 checkerboard (white/black) in RGBA8
            texWidth = 2;
            texHeight = 2;
            pixels = {// row 0: white, black
                      255,
                      255,
                      255,
                      255,
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
        }

        const VkDeviceSize imageSize = static_cast<VkDeviceSize>(pixels.size());

        // Staging buffer
        Buffer staging(m_context->allocator().handle(),
                       m_context->device_handle(),
                       imageSize,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VMA_MEMORY_USAGE_CPU_ONLY);
        {
            void *data = staging.map();
            std::memcpy(data, pixels.data(), static_cast<size_t>(imageSize));
            staging.unmap();
        }

        // Device-local texture
        m_gpu->texture =
            std::make_unique<Texture>(m_context->allocator().handle(),
                                      m_context->device_handle(),
                                      texWidth,
                                      texHeight,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VMA_MEMORY_USAGE_GPU_ONLY,
                                      VK_IMAGE_ASPECT_COLOR_BIT);

        // Upload via UploadContext
        m_gpu->async_uploader->begin();

        // UNDEFINED -> TRANSFER_DST
        m_gpu->async_uploader->transition_image_layout(m_gpu->texture->image(), // VkImage
                                                       VK_IMAGE_ASPECT_COLOR_BIT,
                                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                       /*baseMip*/ 0,
                                                       /*levelCount*/ 1,
                                                       /*baseLayer*/ 0,
                                                       /*layerCount*/ 1);

        // copy staging -> image
        m_gpu->async_uploader->copy_buffer_to_image(
            staging.handle(),
            m_gpu->texture->image(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(m_gpu->texture->width()),
            static_cast<uint32_t>(m_gpu->texture->height()));

        // TRANSFER_DST -> SHADER_READ
        m_gpu->async_uploader->transition_image_layout(m_gpu->texture->image(),
                                                       VK_IMAGE_ASPECT_COLOR_BIT,
                                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                       0,
                                                       1,
                                                       0,
                                                       1);

        UploadTicket t = m_gpu->async_uploader->end_and_submit();

        m_retirement_queue->retire_after(GpuSignal::timeline(t.value),
                                         [st = std::move(staging)]() mutable {});
    }

    void Renderer::create_frames()
    {
        m_gpu->frames.clear();
        m_gpu->frames.reserve(ankh::config().framesInFlight);

        QueueFamilyIndices queues = m_context->queues();
        uint32_t graphicsFamily = queues.graphicsFamily.value();

        VkDeviceSize uboSize = sizeof(FrameUBO);
        VkDeviceSize objectSize = sizeof(ObjectDataGPU) * ankh::config().maxObjects;

        std::vector<VkDescriptorSetLayout> layouts(ankh::config().framesInFlight,
                                                   m_gpu->descriptor_set_layout->handle());

        VkDescriptorSetAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = m_gpu->descriptor_pool->handle();
        ai.descriptorSetCount = ankh::config().framesInFlight;
        ai.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> sets(ankh::config().framesInFlight);

        ANKH_VK_CHECK(vkAllocateDescriptorSets(m_context->device_handle(), &ai, sets.data()));

        for (uint32_t i = 0; i < ankh::config().framesInFlight; ++i)
        {
            // Construct FrameContext
            m_gpu->frames.emplace_back(m_context->allocator().handle(),
                                       m_context->device_handle(),
                                       graphicsFamily,
                                       uboSize,
                                       objectSize,
                                       sets[i],
                                       m_gpu->texture->view(),
                                       m_gpu->texture->sampler(),
                                       m_retirement_queue.get());
        }
    }

    void
    Renderer::record_command_buffer(FrameContext &frame, uint32_t image_index, GpuSignal signal)
    {

        VkCommandBuffer cmd = frame.begin(signal);

        // --- Begin render pass ---
        VkRenderPassBeginInfo rp_info{};
        rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_info.renderPass = m_gpu->render_pass->handle();
        rp_info.framebuffer = m_gpu->swapchain->framebuffer(image_index).handle();
        rp_info.renderArea.offset = {0, 0};
        rp_info.renderArea.extent = m_gpu->swapchain->extent();

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};

        rp_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        rp_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(cmd, &rp_info, VK_SUBPASS_CONTENTS_INLINE);

        // --- Viewport / scissor ---
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_gpu->swapchain->extent().width);
        viewport.height = static_cast<float>(m_gpu->swapchain->extent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_gpu->swapchain->extent();
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        if (!m_gpu->gpu_mesh_pool)
        {
            vkCmdEndRenderPass(cmd);
            frame.end();
            return;
        }

        VkBuffer vb = m_gpu->gpu_mesh_pool->vertex_buffer();
        VkBuffer ib = m_gpu->gpu_mesh_pool->index_buffer();
        const auto &meshInfo = m_gpu->gpu_mesh_pool->draw_info();

        if (vb != VK_NULL_HANDLE && ib != VK_NULL_HANDLE)
        {
            m_gpu->draw_pass
                ->record(cmd, frame, image_index, vb, ib, meshInfo, *m_gpu->scene_renderer);

            m_gpu->ui_pass
                ->record(cmd, frame, image_index, vb, ib, meshInfo, *m_gpu->scene_renderer);
        }

        vkCmdEndRenderPass(cmd);
        frame.end();
    }

    void Renderer::update_uniform_buffer(FrameContext &frame)
    {
        static auto start = std::chrono::high_resolution_clock::now();
        float time =
            std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();

        // 1. Update scene (camera + renderable transforms)
        m_gpu->scene_renderer->update_frame(frame, *m_gpu->swapchain, time);

        // 2. Write FrameUBO
        FrameUBO fubo{};
        const auto &cam = m_gpu->scene_renderer->camera();
        fubo.view = cam.view();
        fubo.proj = cam.proj();
        fubo.globalAlbedo = glm::vec4(1.0f);

        // simple directional light from above-front-left
        glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
        fubo.lightDir = glm::vec4(lightDir, 0.0f);

        std::memcpy(frame.uniform_mapped(), &fubo, sizeof(fubo));

        // 3. Write ObjectDataGPU array
        auto *objData = reinterpret_cast<ObjectDataGPU *>(frame.object_mapped());
        auto &renderables = m_gpu->scene_renderer->renderables();
        auto &materials = m_gpu->scene_renderer->material_pool();

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

            objData[i].model = r.transform;

            glm::vec4 albedo{1.0f};

            if (materials.valid(r.material))
            {
                albedo = materials.get(r.material).albedo();
            }

            objData[i].albedo = albedo;
        }
    }

    void Renderer::draw_frame()
    {
        const FrameSlot slot = m_gpu->frame_ring->current();

        auto &frame = m_gpu->frames[slot];

        VkFence fence = frame.in_flight_fence();
        ANKH_VK_CHECK(vkWaitForFences(m_context->device_handle(), 1, &fence, VK_TRUE, UINT64_MAX));

        m_gpu->gpu_serial->mark_slot_completed(slot);

        m_retirement_queue->collect(m_gpu->gpu_serial->completed(),
                                    m_gpu->async_uploader->completed_value());

        uint32_t image_index = 0;
        VkResult result = vkAcquireNextImageKHR(m_context->device_handle(),
                                                m_gpu->swapchain->handle(),
                                                ankh::config().acquireImageTimeoutNs,
                                                frame.image_available(),
                                                VK_NULL_HANDLE,
                                                &image_index);

        if (result == VK_TIMEOUT)
        {
            ANKH_LOG_WARN("[Renderer] Timed out while acquiring swapchain image; skipping frame");
            return;
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_window->set_framebuffer_resized(false);
            recreate_swapchain();
            return;
        }

        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swapchain image");
        }

        update_uniform_buffer(frame);

        ANKH_VK_CHECK(vkResetFences(m_context->device_handle(), 1, &fence));

        const GpuSerialValue frameId = m_gpu->gpu_serial->issue();

        record_command_buffer(frame, image_index, GpuSignal::frame(frameId));

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

        m_gpu->gpu_serial->mark_slot_used(slot, frameId);

        ANKH_VK_CHECK(vkQueueSubmit(m_context->graphics_queue(), 1, &submit, fence));

        VkPresentInfoKHR present{};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &signalSem;

        VkSwapchainKHR swapchains[] = {m_gpu->swapchain->handle()};
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

        m_gpu->frame_ring->advance();
    }

    void Renderer::recreate_swapchain()
    {
        int width = 0, height = 0;

        glfwGetFramebufferSize(m_window->handle(), &width, &height);

        if (width == 0 || height == 0)
        {
            return;
        }

        retire_swapchain_resources();

        wait_for_all_frames();

        vkQueueWaitIdle(m_context->present_queue());

        cleanup_swapchain();

        m_gpu->swapchain = std::make_unique<Swapchain>(m_context->physical_device(),
                                                       m_context->device_handle(),
                                                       m_context->allocator().handle(),
                                                       m_context->surface_handle(),
                                                       m_window->handle());

        // Recreate render pass with new swapchain format
        m_gpu->render_pass = std::make_unique<RenderPass>(m_context->device_handle(),
                                                          m_gpu->swapchain->image_format());

        // Recreate pipeline layout
        m_gpu->pipeline_layout =
            std::make_unique<PipelineLayout>(m_context->device_handle(),
                                             m_gpu->descriptor_set_layout->handle());

        // Recreate graphics pipeline with new render pass
        m_gpu->graphics_pipeline =
            std::make_unique<GraphicsPipeline>(m_context->device_handle(),
                                               m_gpu->render_pass->handle(),
                                               m_gpu->pipeline_layout->handle());

        // Recreate draw passes with new pipeline references
        m_gpu->draw_pass = std::make_unique<DrawPass>(m_context->device_handle(),
                                                      *m_gpu->swapchain,
                                                      *m_gpu->render_pass,
                                                      *m_gpu->graphics_pipeline,
                                                      *m_gpu->pipeline_layout);

        // Recreate UI pass with new pipeline references
        m_gpu->ui_pass = std::make_unique<UiPass>(m_context->device_handle(),
                                                  *m_gpu->swapchain,
                                                  *m_gpu->render_pass,
                                                  *m_gpu->graphics_pipeline,
                                                  *m_gpu->pipeline_layout);

        // Recreate framebuffers
        create_framebuffers();
    }

    void Renderer::run()
    {
        while (!glfwWindowShouldClose(m_window->handle()))
        {
            glfwPollEvents();
            draw_frame();
        }

        wait_for_all_frames();
    }

    GpuResourceTracker *Renderer::tracker() const
    {
#ifndef NDEBUG
        return m_context ? &m_context->gpu_tracker() : nullptr;
#else
        return nullptr;
#endif
    }

} // namespace ankh
