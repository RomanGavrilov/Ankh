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
#include "sync/sync-primitives.hpp"
#include "utils/types.hpp"
#include "memory/buffer.hpp" // <-- RAII Buffer

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
        vkDeviceWaitIdle(m_device->handle());

        cleanup_swapchain();

        // Uniform buffers: still raw VkBuffer/VkDeviceMemory
        for (size_t i = 0; i < m_uniform_buffers.size(); ++i)
        {
            vkDestroyBuffer(m_device->handle(), m_uniform_buffers[i], nullptr);
            vkFreeMemory(m_device->handle(), m_uniform_buffers_memory[i], nullptr);
        }

        // Descriptor pool is RAII
        delete m_descriptor_pool;

        // Sync is RAII too
        delete m_sync;

        // Index & vertex buffers are RAII via std::unique_ptr<Buffer>
        // -> no vkDestroyBuffer/vkFreeMemory calls needed here
        m_index_buffer.reset();
        m_vertex_buffer.reset();

        vkDestroyCommandPool(m_device->handle(), m_command_pool, nullptr);

        delete m_graphics_pipeline;
        delete m_pipeline_layout;
        delete m_descriptor_set_layout;
        delete m_render_pass;
        delete m_swapchain;

        delete m_device;
        delete m_physical_device;
        delete m_surface;
        delete m_debug_messenger;
        delete m_instance;
        delete m_window;
    }

    void Renderer::init_vulkan()
    {
        m_window = new Window("Vulkan", kWidth, kHeight);
        m_instance = new Instance();
        m_debug_messenger = new DebugMessenger(m_instance->handle());
        m_surface = new Surface(m_instance->handle(), m_window->handle());
        m_physical_device = new PhysicalDevice(m_instance->handle(), m_surface->handle());
        m_device = new Device(*m_physical_device);
        m_swapchain = new Swapchain(*m_physical_device,
                                    m_device->handle(),
                                    m_surface->handle(),
                                    m_window->handle());
        m_render_pass = new RenderPass(m_device->handle(), m_swapchain->image_format());

        m_descriptor_set_layout = new DescriptorSetLayout(m_device->handle());
        m_pipeline_layout = new PipelineLayout(m_device->handle(), m_descriptor_set_layout->handle());
        m_graphics_pipeline = new GraphicsPipeline(m_device->handle(),
                                                   m_render_pass->handle(),
                                                   m_pipeline_layout->handle());

        create_framebuffers();
        create_command_pool();
        create_vertex_buffer();
        create_index_buffer();
        create_uniform_buffers();
        create_descriptor_pool();
        allocate_descriptor_sets();
        create_command_buffers();

        m_sync = new SyncPrimitives(m_device->handle(), kMaxFramesInFlight);
    }

    void Renderer::create_framebuffers()
    {
        for (auto view : m_swapchain->image_views())
        {
            m_framebuffers.push_back(
                new Framebuffer(m_device->handle(),
                                m_render_pass->handle(),
                                view,
                                m_swapchain->extent()));
        }
    }

    void Renderer::cleanup_swapchain()
    {
        for (auto fb : m_framebuffers)
            delete fb;
        m_framebuffers.clear();

        delete m_swapchain;
        m_swapchain = nullptr;
    }

    void Renderer::create_command_pool()
    {
        QueueFamilyIndices indices = m_physical_device->queues();

        VkCommandPoolCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        ci.queueFamilyIndex = indices.graphicsFamily.value();

        if (vkCreateCommandPool(m_device->handle(), &ci, nullptr, &m_command_pool) != VK_SUCCESS)
            throw std::runtime_error("failed to create command pool");
    }

    static uint32_t find_memory_type(VkPhysicalDevice phys,
                                     uint32_t type_filter,
                                     VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProps{};
        vkGetPhysicalDeviceMemoryProperties(phys, &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            if ((type_filter & (1u << i)) &&
                (memProps.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }

        throw std::runtime_error("failed to find suitable memory type");
    }

    void Renderer::create_buffer(VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer &buffer,
                                 VkDeviceMemory &memory)
    {
        VkBufferCreateInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bi.size = size;
        bi.usage = usage;
        bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device->handle(), &bi, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("failed to create buffer");

        VkMemoryRequirements req{};
        vkGetBufferMemoryRequirements(m_device->handle(), buffer, &req);

        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = find_memory_type(m_physical_device->handle(),
                                              req.memoryTypeBits,
                                              properties);

        if (vkAllocateMemory(m_device->handle(), &ai, nullptr, &memory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate buffer memory");

        vkBindBufferMemory(m_device->handle(), buffer, memory, 0);
    }

    void Renderer::copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
    {
        VkCommandBufferAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool = m_command_pool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(m_device->handle(), &ai, &cmd);

        VkCommandBufferBeginInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmd, &bi);

        VkBufferCopy copy{};
        copy.size = size;
        vkCmdCopyBuffer(cmd, src, dst, 1, &copy);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;

        vkQueueSubmit(m_device->graphics_queue(), 1, &submit, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device->graphics_queue());

        vkFreeCommandBuffers(m_device->handle(), m_command_pool, 1, &cmd);
    }

    // ===== changed to use std::unique_ptr<Buffer> =====

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

    // ================================================

    void Renderer::create_uniform_buffers()
    {
        VkDeviceSize size = sizeof(UniformBufferObject);

        m_uniform_buffers.resize(kMaxFramesInFlight);
        m_uniform_buffers_memory.resize(kMaxFramesInFlight);
        m_uniform_buffers_mapped.resize(kMaxFramesInFlight);

        for (int i = 0; i < kMaxFramesInFlight; ++i)
        {
            create_buffer(size,
                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          m_uniform_buffers[i],
                          m_uniform_buffers_memory[i]);

            vkMapMemory(m_device->handle(),
                        m_uniform_buffers_memory[i],
                        0,
                        size,
                        0,
                        &m_uniform_buffers_mapped[i]);
        }
    }

    void Renderer::create_descriptor_pool()
    {
        m_descriptor_pool = new DescriptorPool(m_device->handle(), kMaxFramesInFlight);
    }

    void Renderer::allocate_descriptor_sets()
    {
        std::vector<VkDescriptorSetLayout> layouts(kMaxFramesInFlight,
                                                   m_descriptor_set_layout->handle());

        VkDescriptorSetAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = m_descriptor_pool->handle();
        ai.descriptorSetCount = kMaxFramesInFlight;
        ai.pSetLayouts = layouts.data();

        m_descriptor_sets.resize(kMaxFramesInFlight);
        if (vkAllocateDescriptorSets(m_device->handle(), &ai, m_descriptor_sets.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate descriptor sets");

        for (int i = 0; i < kMaxFramesInFlight; ++i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniform_buffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = m_descriptor_sets[i];
            write.dstBinding = 0;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.descriptorCount = 1;
            write.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(m_device->handle(), 1, &write, 0, nullptr);
        }
    }

    void Renderer::create_command_buffers()
    {
        m_command_buffers.resize(kMaxFramesInFlight);

        VkCommandBufferAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool = m_command_pool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size());

        if (vkAllocateCommandBuffers(m_device->handle(), &ai, m_command_buffers.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers");
    }

    void Renderer::record_command_buffer(VkCommandBuffer cmd, uint32_t image_index)
    {
        VkCommandBufferBeginInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(cmd, &bi) != VK_SUCCESS)
            throw std::runtime_error("failed to begin command buffer");

        VkRenderPassBeginInfo rp{};
        rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp.renderPass = m_render_pass->handle();
        rp.framebuffer = m_framebuffers[image_index]->handle();
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

        vkCmdBindDescriptorSets(cmd,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipeline_layout->handle(),
                                0,
                                1,
                                &m_descriptor_sets[m_current_frame],
                                0,
                                nullptr);

        vkCmdDrawIndexed(cmd, static_cast<uint32_t>(kIndices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(cmd);

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer");
    }

    void Renderer::update_uniform_buffer(uint32_t current_image)
    {
        static auto start = std::chrono::high_resolution_clock::now();

        float time = std::chrono::duration<float>(
                         std::chrono::high_resolution_clock::now() - start)
                         .count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f),
                                time * glm::radians(5.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                               glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f),
                                    m_swapchain->extent().width /
                                        static_cast<float>(m_swapchain->extent().height),
                                    0.1f,
                                    10.0f);
        ubo.proj[1][1] *= -1;

        std::memcpy(m_uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
    }

    void Renderer::draw_frame()
    {
        auto &fences = m_sync->in_flight_fences();
        auto &imgAvail = m_sync->image_available();
        auto &renderFin = m_sync->render_finished();

        vkWaitForFences(m_device->handle(), 1, &fences[m_current_frame], VK_TRUE, UINT64_MAX);

        uint32_t image_index = 0;
        VkResult result = vkAcquireNextImageKHR(m_device->handle(),
                                                m_swapchain->handle(),
                                                UINT64_MAX,
                                                imgAvail[m_current_frame],
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

        update_uniform_buffer(m_current_frame);

        vkResetFences(m_device->handle(), 1, &fences[m_current_frame]);

        vkResetCommandBuffer(m_command_buffers[m_current_frame], 0);
        record_command_buffer(m_command_buffers[m_current_frame], image_index);

        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &imgAvail[m_current_frame];
        submit.pWaitDstStageMask = waitStages;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &m_command_buffers[m_current_frame];
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &renderFin[m_current_frame];

        if (vkQueueSubmit(m_device->graphics_queue(), 1, &submit, fences[m_current_frame]) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer");

        VkPresentInfoKHR present{};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &renderFin[m_current_frame];

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

        m_swapchain = new Swapchain(*m_physical_device,
                                    m_device->handle(),
                                    m_surface->handle(),
                                    m_window->handle());
        m_render_pass = new RenderPass(m_device->handle(), m_swapchain->image_format());
        create_framebuffers();
    }

    void Renderer::run()
    {
        while (!glfwWindowShouldClose(m_window->handle()))
        {
            glfwPollEvents();
            draw_frame();
        }

        vkDeviceWaitIdle(m_device->handle());
    }

} // namespace ankh
