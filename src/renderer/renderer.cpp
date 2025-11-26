// src/renderer/renderer.cpp

#include "renderer/renderer.hpp"

#include "platform/window.hpp"
#include "platform/surface.hpp"
#include "core/instance.hpp"
#include "core/debug-messenger.hpp"
#include "core/physical-device.hpp"
#include "core/device.hpp"
#include "swapchain/swapchain.hpp"
#include "renderpass/render-pass.hpp"
#include "renderpass/frame-buffer.hpp"
#include "descriptors/descriptor-set-layout.hpp"
#include "descriptors/descriptor-pool.hpp"
#include "pipeline/pipeline-layout.hpp"
#include "pipeline/graphics-pipeline.hpp"
#include "utils/types.hpp"
#include "memory/buffer.hpp"
#include "commands/command-pool.hpp"
#include "commands/command-buffer.hpp"
#include "frame/frame-context.hpp"

#include <chrono>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace ankh
{

    Renderer::Renderer()
    {
        init_vulkan();
    }

    Renderer::~Renderer()
    {
        // Make sure nothing is in flight
        if (m_device)
        {
            vkDeviceWaitIdle(m_device->handle());
        }

        // Destroy per-frame contexts first (command buffers, pools, UBOs, sync)
        m_frames.clear();

        // Destroy swapchain-dependent stuff
        cleanup_swapchain();

        // Descriptor pool (descriptor sets implicitly freed)
        m_descriptor_pool.reset();

        // Geometry buffers
        m_index_buffer.reset();
        m_vertex_buffer.reset();

        // Pipeline and layout
        m_graphics_pipeline.reset();
        m_pipeline_layout.reset();

        // Descriptor layout
        m_descriptor_set_layout.reset();

        // Render pass, swapchain, device, etc.
        m_render_pass.reset();
        m_swapchain.reset();
        m_device.reset();

        if (m_physical_device)
        {
            delete m_physical_device;
            m_physical_device = nullptr;
        }

        m_surface.reset();
        m_debug_messenger.reset();
        m_instance.reset();
        m_window.reset();
    }

    void Renderer::init_vulkan()
    {
        m_window = std::make_unique<Window>("Vulkan", kWidth, kHeight);
        m_instance = std::make_unique<Instance>();
        m_debug_messenger = std::make_unique<DebugMessenger>(m_instance->handle());
        m_surface = std::make_unique<Surface>(m_instance->handle(), m_window->handle());
        m_physical_device = new PhysicalDevice(m_instance->handle(), m_surface->handle());
        m_device = std::make_unique<Device>(*m_physical_device);

        m_swapchain = std::make_unique<Swapchain>(*m_physical_device,
                                                  m_device->handle(),
                                                  m_surface->handle(),
                                                  m_window->handle());

        m_render_pass = std::make_unique<RenderPass>(m_device->handle(), m_swapchain->image_format());

        m_descriptor_set_layout = std::make_unique<DescriptorSetLayout>(m_device->handle());
        m_pipeline_layout = std::make_unique<PipelineLayout>(m_device->handle(), m_descriptor_set_layout->handle());
        m_graphics_pipeline = std::make_unique<GraphicsPipeline>(m_device->handle(),
                                                                 m_render_pass->handle(),
                                                                 m_pipeline_layout->handle());

        create_framebuffers();
        create_vertex_buffer();
        create_index_buffer();
        create_descriptor_pool();
        create_frames();
    }

    void Renderer::create_framebuffers()
    {
        m_framebuffers.clear();
        m_framebuffers.reserve(m_swapchain->image_views().size());

        for (auto view : m_swapchain->image_views())
        {
            m_framebuffers.emplace_back(
                m_device->handle(),
                m_render_pass->handle(),
                view,
                m_swapchain->extent());
        }
    }

    void Renderer::cleanup_swapchain()
    {
        m_framebuffers.clear();
        m_swapchain.reset();
        m_render_pass.reset();
    }

    // ==== single-use copy using a temporary pool + command buffer ====

    void Renderer::copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
    {
        QueueFamilyIndices indices = m_physical_device->queues();
        uint32_t queueFamily = indices.graphicsFamily.value();

        // Temporary per-copy pool + command buffer
        CommandPool pool(m_device->handle(), queueFamily);
        CommandBuffer cmd(m_device->handle(), pool.handle());

        cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkBufferCopy copy{};
        copy.size = size;
        vkCmdCopyBuffer(cmd.handle(), src, dst, 1, &copy);

        cmd.end();

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkCommandBuffer raw = cmd.handle();
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &raw;

        vkQueueSubmit(m_device->graphics_queue(), 1, &submit, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device->graphics_queue());
        // CommandBuffer / CommandPool are freed automatically via RAII
    }

    // ===== vertex/index buffers using Buffer RAII =====

    void Renderer::create_vertex_buffer()
    {
        VkDeviceSize size = sizeof(Vertex) * kVertices.size();

        // Staging buffer in host-visible memory
        Buffer staging(
            m_physical_device->handle(),
            m_device->handle(),
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            void *data = staging.map(0, size);
            std::memcpy(data, kVertices.data(), static_cast<size_t>(size));
            staging.unmap();
        }

        // Device-local vertex buffer (owned via unique_ptr)
        m_vertex_buffer = std::make_unique<Buffer>(
            m_physical_device->handle(),
            m_device->handle(),
            size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        copy_buffer(staging.handle(), m_vertex_buffer->handle(), size);
    }

    void Renderer::create_index_buffer()
    {
        VkDeviceSize size = sizeof(uint16_t) * kIndices.size();

        Buffer staging(
            m_physical_device->handle(),
            m_device->handle(),
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            void *data = staging.map(0, size);
            std::memcpy(data, kIndices.data(), static_cast<size_t>(size));
            staging.unmap();
        }

        m_index_buffer = std::make_unique<Buffer>(
            m_physical_device->handle(),
            m_device->handle(),
            size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        copy_buffer(staging.handle(), m_index_buffer->handle(), size);
    }

    // ===== descriptor pool + sets =====

    void Renderer::create_descriptor_pool()
    {
        m_descriptor_pool = std::make_unique<DescriptorPool>(m_device->handle(), kMaxFramesInFlight);
    }

    // ===== create per-frame contexts (pool + cmd buffer + UBO + sync) =====

    void Renderer::create_frames()
    {
        m_frames.clear();
        m_frames.reserve(kMaxFramesInFlight);

        QueueFamilyIndices queues = m_physical_device->queues();
        uint32_t graphicsFamily = queues.graphicsFamily.value();
        VkDeviceSize uboSize = sizeof(UniformBufferObject);

        // 1) Allocate descriptor sets (one per frame)
        std::vector<VkDescriptorSetLayout> layouts(
            kMaxFramesInFlight,
            m_descriptor_set_layout->handle());

        VkDescriptorSetAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = m_descriptor_pool->handle();
        ai.descriptorSetCount = kMaxFramesInFlight;
        ai.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> sets(kMaxFramesInFlight);
        if (vkAllocateDescriptorSets(m_device->handle(), &ai, sets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets");
        }

        // 2) Create one FrameContext per frame, each owning its descriptor set
        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            m_frames.emplace_back(
                m_physical_device->handle(),
                m_device->handle(),
                graphicsFamily,
                uboSize,
                sets[i]);
        }
    }

    // ===== command buffer recording =====

    void Renderer::record_command_buffer(VkCommandBuffer cmd, uint32_t image_index)
    {
        if (image_index >= m_framebuffers.size())
        {
            throw std::runtime_error("invalid framebuffer index in record_command_buffer");
        }

        VkCommandBufferBeginInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(cmd, &bi) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin command buffer");
        }

        VkRenderPassBeginInfo rp{};
        rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp.renderPass = m_render_pass->handle();
        rp.framebuffer = m_framebuffers[image_index].handle();
        rp.renderArea.offset = {0, 0};
        rp.renderArea.extent = m_swapchain->extent();

        VkClearValue clear{};
        clear.color = {{0.f, 0.f, 0.f, 1.f}};
        rp.clearValueCount = 1;
        rp.pClearValues = &clear;

        vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline->handle());

        VkViewport vp{};
        vp.x = 0.f;
        vp.y = 0.f;
        vp.width = static_cast<float>(m_swapchain->extent().width);
        vp.height = static_cast<float>(m_swapchain->extent().height);
        vp.minDepth = 0.f;
        vp.maxDepth = 1.f;
        vkCmdSetViewport(cmd, 0, 1, &vp);

        VkRect2D sc{};
        sc.offset = {0, 0};
        sc.extent = m_swapchain->extent();
        vkCmdSetScissor(cmd, 0, 1, &sc);

        VkBuffer vertexBuffers[] = {m_vertex_buffer->handle()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(cmd, m_index_buffer->handle(), 0, VK_INDEX_TYPE_UINT16);

        VkDescriptorSet set = m_frames[m_current_frame].descriptor_set();
        vkCmdBindDescriptorSets(cmd,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipeline_layout->handle(),
                                0,
                                1,
                                &set,
                                0,
                                nullptr);

        vkCmdDrawIndexed(cmd, static_cast<uint32_t>(kIndices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(cmd);

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer");
    }

    // ===== UBO update using FrameContext's mapped pointer =====

    void Renderer::update_uniform_buffer(FrameContext &frame)
    {
        static auto start = std::chrono::high_resolution_clock::now();

        float time = std::chrono::duration<float>(
                         std::chrono::high_resolution_clock::now() - start)
                         .count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f),
                                time * glm::radians(5.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));

        // look at the origin from (2,2,2)
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                               glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 0.0f, 1.0f));

        ubo.proj = glm::perspective(glm::radians(45.0f),
                                    m_swapchain->extent().width /
                                        static_cast<float>(m_swapchain->extent().height),
                                    0.1f,
                                    10.0f);
        ubo.proj[1][1] *= -1;

        std::memcpy(frame.uniform_mapped(), &ubo, sizeof(ubo));
    }

    // ===== main frame loop using FrameContext =====

    void Renderer::draw_frame()
    {
        auto &frame = m_frames[m_current_frame];

        VkFence fence = frame.in_flight_fence();

        // Wait until this frame's command buffer is not in flight
        vkWaitForFences(m_device->handle(), 1, &fence, VK_TRUE, UINT64_MAX);

        uint32_t image_index = 0;
        VkResult result = vkAcquireNextImageKHR(m_device->handle(),
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

        vkResetFences(m_device->handle(), 1, &fence);

        VkCommandBuffer cmd = frame.command_buffer();
        vkResetCommandBuffer(cmd, 0);
        record_command_buffer(cmd, image_index);

        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

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

        if (vkQueueSubmit(m_device->graphics_queue(), 1, &submit, fence) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer");

        VkPresentInfoKHR present{};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &signalSem;

        VkSwapchainKHR swapchains[] = {m_swapchain->handle()};
        present.swapchainCount = 1;
        present.pSwapchains = swapchains;
        present.pImageIndices = &image_index;

        result = vkQueuePresentKHR(m_device->present_queue(), &present);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebuffer_resized)
        {
            m_framebuffer_resized = false;
            recreate_swapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swapchain image");
        }

        m_current_frame = (m_current_frame + 1) % kMaxFramesInFlight;
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

        vkDeviceWaitIdle(m_device->handle());

        cleanup_swapchain();

        m_swapchain = std::make_unique<Swapchain>(*m_physical_device,
                                                  m_device->handle(),
                                                  m_surface->handle(),
                                                  m_window->handle());
        m_render_pass = std::make_unique<RenderPass>(m_device->handle(), m_swapchain->image_format());
        create_framebuffers();
        // FrameContexts remain valid: they only depend on device/queue/UBO/descriptor sets
    }

    void Renderer::run()
    {
        while (!glfwWindowShouldClose(m_window->handle()))
        {
            glfwPollEvents();
            draw_frame();
            m_frames_rendered++;
        }

        vkDeviceWaitIdle(m_device->handle());
    }

    bool Renderer::run_frames(uint32_t frame_count)
    {
        for (uint32_t i = 0; i < frame_count && !glfwWindowShouldClose(m_window->handle()); ++i)
        {
            glfwPollEvents();
            draw_frame();
            m_frames_rendered++;
        }

        vkDeviceWaitIdle(m_device->handle());
        return true;
    }

} // namespace ankh
