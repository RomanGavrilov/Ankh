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
#include "streaming/async-uploader.hpp"
#include "utils/types.hpp"
#include <vk_mem_alloc.h>

namespace ankh
{
    class GpuMeshPool
    {
      public:
        GpuMeshPool(VmaAllocator allocator,
                    VkDevice device,
                    AsyncUploader &uploadContext,
                    GpuRetirementQueue *retirement);

        // Build unified buffers from all valid meshes in the MeshPool.
        // Call this after meshes are loaded.
        void build_from_mesh_pool(const MeshPool &mesh_pool);

        void mark_used(GpuSignal signal) noexcept;

        VkBuffer vertex_buffer() const noexcept;

        VkBuffer index_buffer() const noexcept;

        const std::unordered_map<MeshHandle, MeshDrawInfo> &draw_info() const noexcept;

      private:
        VkDevice m_device{VK_NULL_HANDLE};
        VmaAllocator m_allocator{VK_NULL_HANDLE};
        AsyncUploader &m_async_uploader;

        std::unique_ptr<Buffer> m_vertex_buffer;
        std::unique_ptr<Buffer> m_index_buffer;
        std::unordered_map<MeshHandle, MeshDrawInfo> m_draw_info;

        GpuRetirementQueue *m_retirement{nullptr};
    };

} // namespace ankh
