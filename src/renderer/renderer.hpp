// src/renderer/renderer.hpp
#pragma once

#include "utils/types.hpp"

#include <memory>
#include <vector>

namespace ankh
{

    class Window;
    class Instance;
    class DebugMessenger;
    class PhysicalDevice;
    class Device;
    class Surface;
    class Swapchain;
    class RenderPass;
    class DescriptorSetLayout;
    class DescriptorPool;
    class PipelineLayout;
    class GraphicsPipeline;
    class Framebuffer;
    class Buffer;
    class FrameContext;
    class UploadContext; // ⬅️ new forward declaration

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        void run();

    private:
        void init_vulkan();
        void create_framebuffers();

        void create_vertex_buffer();
        void create_index_buffer();

        void create_descriptor_pool();
        void create_frames();

        void record_command_buffer(VkCommandBuffer cmd, uint32_t image_index);
        void update_uniform_buffer(FrameContext &frame);

        void draw_frame();
        void recreate_swapchain();
        void cleanup_swapchain();

    private:
        std::unique_ptr<Window> m_window;
        std::unique_ptr<Instance> m_instance;
        std::unique_ptr<DebugMessenger> m_debug_messenger;
        std::unique_ptr<Surface> m_surface;
        PhysicalDevice *m_physical_device = nullptr;
        std::unique_ptr<Device> m_device;

        std::unique_ptr<Swapchain> m_swapchain;
        std::unique_ptr<RenderPass> m_render_pass;

        std::unique_ptr<DescriptorSetLayout> m_descriptor_set_layout;
        std::unique_ptr<DescriptorPool> m_descriptor_pool;
        std::unique_ptr<PipelineLayout> m_pipeline_layout;
        std::unique_ptr<GraphicsPipeline> m_graphics_pipeline;

        std::unique_ptr<Buffer> m_vertex_buffer{};
        std::unique_ptr<Buffer> m_index_buffer{};

        std::vector<FrameContext> m_frames;

        uint32_t m_current_frame = 0;
        bool m_framebuffer_resized = false;

        std::unique_ptr<UploadContext> m_upload_context;
    };

} // namespace ankh
