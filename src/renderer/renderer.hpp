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

    struct RendererGpuState
    {
        std::vector<FrameContext> frames;
        std::unique_ptr<Texture> texture;
        std::unique_ptr<GpuMeshPool> gpu_mesh_pool;

        std::unique_ptr<UiPass> ui_pass;
        std::unique_ptr<DrawPass> draw_pass;
        std::unique_ptr<GraphicsPipeline> graphics_pipeline;
        std::unique_ptr<PipelineLayout> pipeline_layout;
        std::unique_ptr<RenderPass> render_pass;
        std::unique_ptr<Swapchain> swapchain;

        std::unique_ptr<AsyncUploader> async_uploader;
        std::unique_ptr<DescriptorPool> descriptor_pool;
        std::unique_ptr<DescriptorSetLayout> descriptor_set_layout;

        std::unique_ptr<SceneRenderer> scene_renderer;
        std::unique_ptr<FrameRing> frame_ring;
        std::unique_ptr<GpuSerial> gpu_serial;
    };

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
        std::unique_ptr<Context> m_context;

        std::unique_ptr<GpuRetirementQueue> m_retirement_queue;

        std::unique_ptr<RendererGpuState> m_gpu;

        std::unique_ptr<Window> m_window;
    };

} // namespace ankh
