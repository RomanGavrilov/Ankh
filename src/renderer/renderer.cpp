// src/renderer/renderer.cpp

#include "renderer/renderer.hpp"

#include "platform/window.hpp"

#include "core/context.hpp"

#include "swapchain/swapchain.hpp"

#include "renderpass/frame-buffer.hpp"
#include "renderpass/render-pass.hpp"

#include "draw-pass.hpp"

#include "descriptors/descriptor-pool.hpp"
#include "descriptors/descriptor-set-layout.hpp"

#include "pipeline/graphics-pipeline.hpp"
#include "pipeline/pipeline-layout.hpp"

#include "utils/types.hpp"

#include "memory/buffer.hpp"
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

    Renderer::Renderer() { init_vulkan(); }

    Renderer::~Renderer()
    {
        if (m_context)
        {
            vkDeviceWaitIdle(m_context->device_handle());
        }

        m_frames.clear();

        cleanup_swapchain();

        m_descriptor_pool.reset();
        m_index_buffer.reset();
        m_vertex_buffer.reset();

        m_graphics_pipeline.reset();
        m_pipeline_layout.reset();
        m_descriptor_set_layout.reset();

        m_draw_pass.reset();
        m_upload_context.reset();

        m_context.reset();
        m_window.reset();
    }

    void Renderer::init_vulkan()
    {
        m_window = std::make_unique<Window>("Vulkan", kWidth, kHeight);

        // Context sets up instance, debug, surface, physical device, device
        m_context = std::make_unique<Context>(m_window->handle());

        // Upload context: device + graphics queue family index
        m_upload_context = std::make_unique<UploadContext>(m_context->device_handle(), m_context->queues().graphicsFamily.value());

        m_swapchain = std::make_unique<Swapchain>(m_context->physical_device(),
                                                  m_context->device_handle(),
                                                  m_context->surface_handle(),
                                                  m_window->handle());

        m_render_pass = std::make_unique<RenderPass>(m_context->device_handle(), m_swapchain->image_format());

        m_descriptor_set_layout = std::make_unique<DescriptorSetLayout>(m_context->device_handle());

        m_pipeline_layout = std::make_unique<PipelineLayout>(m_context->device_handle(), m_descriptor_set_layout->handle());

        m_graphics_pipeline =
            std::make_unique<GraphicsPipeline>(m_context->device_handle(), m_render_pass->handle(), m_pipeline_layout->handle());

        m_draw_pass =
            std::make_unique<DrawPass>(m_context->device_handle(), *m_swapchain, *m_render_pass, *m_graphics_pipeline, *m_pipeline_layout);

        create_framebuffers();
        create_vertex_buffer();
        create_index_buffer();
        create_descriptor_pool();
        create_frames();

        m_frame_sync = std::make_unique<FrameSync>(kMaxFramesInFlight);
    }

    void Renderer::create_framebuffers() { m_swapchain->create_framebuffers(m_render_pass->handle()); }

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
        VkDeviceSize size = sizeof(Vertex) * kVertices.size();

        Buffer staging(m_context->physical_device().handle(),
                       m_context->device_handle(),
                       size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            void *data = staging.map(0, size);
            std::memcpy(data, kVertices.data(), static_cast<size_t>(size));
            staging.unmap();
        }

        m_vertex_buffer = std::make_unique<Buffer>(m_context->physical_device().handle(),
                                                   m_context->device_handle(),
                                                   size,
                                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_upload_context->copy_buffer(m_context->graphics_queue(), staging.handle(), m_vertex_buffer->handle(), size);
    }

    void Renderer::create_index_buffer()
    {
        VkDeviceSize size = sizeof(uint16_t) * kIndices.size();

        Buffer staging(m_context->physical_device().handle(),
                       m_context->device_handle(),
                       size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            void *data = staging.map(0, size);
            std::memcpy(data, kIndices.data(), static_cast<size_t>(size));
            staging.unmap();
        }

        m_index_buffer = std::make_unique<Buffer>(m_context->physical_device().handle(),
                                                  m_context->device_handle(),
                                                  size,
                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_upload_context->copy_buffer(m_context->graphics_queue(), staging.handle(), m_index_buffer->handle(), size);
    }

    void Renderer::create_descriptor_pool()
    {
        m_descriptor_pool = std::make_unique<DescriptorPool>(m_context->device_handle(), kMaxFramesInFlight);
    }

    void Renderer::create_frames()
    {
        m_frames.clear();
        m_frames.reserve(kMaxFramesInFlight);

        QueueFamilyIndices queues = m_context->queues();
        uint32_t graphicsFamily = queues.graphicsFamily.value();
        VkDeviceSize uboSize = sizeof(UniformBufferObject);

        std::vector<VkDescriptorSetLayout> layouts(kMaxFramesInFlight, m_descriptor_set_layout->handle());

        VkDescriptorSetAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = m_descriptor_pool->handle();
        ai.descriptorSetCount = kMaxFramesInFlight;
        ai.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> sets(kMaxFramesInFlight);

        ANKH_VK_CHECK(vkAllocateDescriptorSets(m_context->device_handle(), &ai, sets.data()));

        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            m_frames.emplace_back(m_context->physical_device().handle(), m_context->device_handle(), graphicsFamily, uboSize, sets[i]);
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

        VkClearValue clear_value{};
        clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
        rp_info.clearValueCount = 1;
        rp_info.pClearValues = &clear_value;

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
        const uint32_t index_count = static_cast<uint32_t>(kIndices.size());
        m_draw_pass->record(cmd, frame, image_index, m_vertex_buffer->handle(), m_index_buffer->handle(), index_count);

        // Add additional passes (UI, debug, etc.) can be recorded here
        // using the same cmd and render pass.

        // --- End render pass + command buffer ---
        vkCmdEndRenderPass(cmd);
        frame.end();
    }

    void Renderer::update_uniform_buffer(FrameContext &frame)
    {
        static auto start = std::chrono::high_resolution_clock::now();

        float time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f),
                                    m_swapchain->extent().width / static_cast<float>(m_swapchain->extent().height),
                                    0.1f,
                                    10.0f);
        ubo.proj[1][1] *= -1;

        std::memcpy(frame.uniform_mapped(), &ubo, sizeof(ubo));
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

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebuffer_resized)
        {
            m_framebuffer_resized = false;
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

        m_render_pass = std::make_unique<RenderPass>(m_context->device_handle(), m_swapchain->image_format());

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
