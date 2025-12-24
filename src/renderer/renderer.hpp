// src/renderer/renderer.hpp
#pragma once

#include "scene/renderable.hpp"
#include "utils/config.hpp"
#include "utils/types.hpp"

#include <memory>
#include <unordered_map>
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
    class AsyncUploader;
    class DrawPass;
    class FrameSync;
    class UiPass;
    class SceneRenderer;
    class GpuMeshPool;
    class Texture;
    class DeferredDeletionQueue;
    class GpuResourceTracker;
    class FrameRing;
    class GpuSerial;
    class GpuRetirementQueue;
    class GpuSignal;

    class Renderer
    {
      public:
        Renderer();
        ~Renderer();

        void run();

      private:
        void init_vulkan();
        void create_framebuffers();

        void create_descriptor_pool();
        void create_texture();
        void create_frames();

        void record_command_buffer(FrameContext &frame, uint32_t image_index, GpuSignal signal);
        void update_uniform_buffer(FrameContext &frame);

        void draw_frame();
        void recreate_swapchain();
        void cleanup_swapchain();
        void wait_for_all_frames();
        void retire_swapchain_resources();
        GpuResourceTracker *tracker() const;

      private:
        std::unique_ptr<GpuRetirementQueue> m_retirement_queue;

        std::unique_ptr<Window> m_window;
        std::unique_ptr<Context> m_context;
        std::unique_ptr<Swapchain> m_swapchain;
        std::unique_ptr<RenderPass> m_render_pass;
        std::unique_ptr<DescriptorSetLayout> m_descriptor_set_layout;
        std::unique_ptr<DescriptorPool> m_descriptor_pool;
        std::unique_ptr<PipelineLayout> m_pipeline_layout;
        std::unique_ptr<GraphicsPipeline> m_graphics_pipeline;
        std::unique_ptr<AsyncUploader> m_async_uploader;
        std::unique_ptr<DrawPass> m_draw_pass;
        std::unique_ptr<UiPass> m_ui_pass;
        std::unique_ptr<SceneRenderer> m_scene_renderer;

        std::unique_ptr<Texture> m_texture;

        std::unique_ptr<FrameRing> m_frame_ring;
        std::unique_ptr<GpuSerial> m_gpu_serial;

        std::vector<FrameContext> m_frames;

        std::unique_ptr<GpuMeshPool> m_gpu_mesh_pool;
    };

} // namespace ankh
