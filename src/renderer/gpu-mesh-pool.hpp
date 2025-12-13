// src/renderer/gpu-mesh-pool.hpp
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "memory/buffer.hpp"
#include "memory/upload-context.hpp"
#include "renderer/mesh-draw-info.hpp"
#include "scene/mesh-pool.hpp"
#include "scene/renderable.hpp" // MeshHandle
#include "utils/types.hpp"
#include <vk_mem_alloc.h>

namespace ankh
{
    class GpuMeshPool
    {
      public:
        GpuMeshPool(VmaAllocator allocator, VkDevice device, UploadContext &uploadContext);

        // Build unified buffers from all valid meshes in the MeshPool.
        // Call this after meshes are loaded.
        void build_from_mesh_pool(const MeshPool &mesh_pool, VkQueue graphicsQueue);

        VkBuffer vertex_buffer() const
        {
            return m_vertex_buffer ? m_vertex_buffer->handle() : VK_NULL_HANDLE;
        }

        VkBuffer index_buffer() const
        {
            return m_index_buffer ? m_index_buffer->handle() : VK_NULL_HANDLE;
        }

        const std::unordered_map<MeshHandle, MeshDrawInfo> &draw_info() const
        {
            return m_draw_info;
        }

      private:
        
        VkDevice m_device{VK_NULL_HANDLE};
        VmaAllocator m_allocator{VK_NULL_HANDLE};
        UploadContext &m_upload_context;

        std::unique_ptr<Buffer> m_vertex_buffer;
        std::unique_ptr<Buffer> m_index_buffer;
        std::unordered_map<MeshHandle, MeshDrawInfo> m_draw_info;
    };

} // namespace ankh
