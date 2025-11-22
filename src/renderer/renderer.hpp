#pragma once
#include "utils/types.hpp"
#include "memory/buffer.hpp"
#include <memory>

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
    class SyncPrimitives;
    class Framebuffer;
    class Buffer;

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        void run();

    private:
        void init_vulkan();
        void create_framebuffers();
        void create_command_pool();
        void create_vertex_buffer();
        void create_index_buffer();
        void create_uniform_buffers();
        void create_descriptor_pool();
        void allocate_descriptor_sets();
        void create_command_buffers();
        void record_command_buffer(VkCommandBuffer cmd, uint32_t image_index);
        void update_uniform_buffer(uint32_t current_image);

        void create_buffer(VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties,
                           VkBuffer &buffer,
                           VkDeviceMemory &memory);

        void copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

        void draw_frame();
        void recreate_swapchain();
        void cleanup_swapchain();

    private:
        Window *m_window = nullptr;
        Instance *m_instance = nullptr;
        DebugMessenger *m_debug_messenger = nullptr;
        Surface *m_surface = nullptr;
        PhysicalDevice *m_physical_device = nullptr;
        Device *m_device = nullptr;
        Swapchain *m_swapchain = nullptr;
        RenderPass *m_render_pass = nullptr;

        DescriptorSetLayout *m_descriptor_set_layout = nullptr;
        DescriptorPool *m_descriptor_pool = nullptr;
        PipelineLayout *m_pipeline_layout = nullptr;
        GraphicsPipeline *m_graphics_pipeline = nullptr;

        std::vector<Framebuffer *> m_framebuffers;

        VkCommandPool m_command_pool{};
        std::vector<VkCommandBuffer> m_command_buffers;

        std::unique_ptr<Buffer> m_vertex_buffer{};
        
        std::unique_ptr<Buffer> m_index_buffer{};
       

        std::vector<VkBuffer> m_uniform_buffers;
        std::vector<VkDeviceMemory> m_uniform_buffers_memory;
        std::vector<void *> m_uniform_buffers_mapped;

        std::vector<VkDescriptorSet> m_descriptor_sets;

        SyncPrimitives *m_sync = nullptr;
        uint32_t m_current_frame = 0;

        bool m_framebuffer_resized = false;
    };

} // namespace ankh
