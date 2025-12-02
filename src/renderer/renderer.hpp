// src/renderer/renderer.hpp
#pragma once

#include "utils/types.hpp"

#include <memory>
#include <vector>

namespace ankh
{

    class Window;
    class Context;
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
    class UploadContext;
    class DrawPass;
    class FrameSync;
    class UiPass;
    class SceneRenderer;

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

        void record_command_buffer(FrameContext &frame, uint32_t image_index);
        void update_uniform_buffer(FrameContext &frame);

        void draw_frame();
        void recreate_swapchain();
        void cleanup_swapchain();

      private:
        std::unique_ptr<Window> m_window;
        std::unique_ptr<Context> m_context;
        std::unique_ptr<Swapchain> m_swapchain;
        std::unique_ptr<RenderPass> m_render_pass;
        std::unique_ptr<DescriptorSetLayout> m_descriptor_set_layout;
        std::unique_ptr<DescriptorPool> m_descriptor_pool;
        std::unique_ptr<PipelineLayout> m_pipeline_layout;
        std::unique_ptr<GraphicsPipeline> m_graphics_pipeline;
        std::unique_ptr<Buffer> m_vertex_buffer{};
        std::unique_ptr<Buffer> m_index_buffer{};
        std::vector<FrameContext> m_frames;
        std::unique_ptr<UploadContext> m_upload_context;
        std::unique_ptr<DrawPass> m_draw_pass;
        std::unique_ptr<UiPass> m_ui_pass;
        std::unique_ptr<SceneRenderer> m_scene_renderer;
        std::unique_ptr<FrameSync> m_frame_sync;
        bool m_framebuffer_resized = false;
    };

} // namespace ankh
