// src/renderer/gpu-mesh-pool.cpp
#include "renderer/gpu-mesh-pool.hpp"
#include "streaming/async-uploader.hpp"

#include <cstring>
#include <utils/logging.hpp>

namespace ankh
{

    GpuMeshPool::GpuMeshPool(VmaAllocator allocator,
                             VkDevice device,
                             AsyncUploader &uploadContext,
                             GpuRetirementQueue *retirement)
        : m_allocator{allocator}
        , m_device{device}
        , m_async_uploader{uploadContext}
        , m_retirement{retirement}
    {
    }

    void GpuMeshPool::build_from_mesh_pool(const MeshPool &mesh_pool)
    {
        std::vector<Vertex> allVertices;
        std::vector<uint16_t> allIndices;
        allVertices.reserve(1024);
        allIndices.reserve(1024);

        m_draw_info.clear();

        const auto handles = mesh_pool.handles();

        for (MeshHandle h : handles)
        {
            const Mesh &mesh = mesh_pool.get(h);

            const auto &verts = mesh.vertices();
            const auto &indices = mesh.indices();

            MeshDrawInfo info{};
            info.firstIndex = static_cast<uint32_t>(allIndices.size());
            info.indexCount = static_cast<uint32_t>(indices.size());
            info.vertexOffset = static_cast<int32_t>(allVertices.size());

            allVertices.insert(allVertices.end(), verts.begin(), verts.end());
            allIndices.insert(allIndices.end(), indices.begin(), indices.end());

            m_draw_info[h] = info;
        }

        if (allVertices.empty() || allIndices.empty())
        {
            ANKH_LOG_WARN("[GpuMeshPool] BuildFromMeshPool: no mesh data to upload.");
            m_vertex_buffer.reset();
            m_index_buffer.reset();
            return;
        }

        VkDeviceSize vertexBufferSize = sizeof(Vertex) * allVertices.size();
        VkDeviceSize indexBufferSize = sizeof(uint16_t) * allIndices.size();

        // -----------------------------
        // Vertex staging + device-local buffer
        // -----------------------------
        // Vertex upload (staging -> GPU)
        // -----------------------------
        Buffer vertexStaging(m_allocator,
                             m_device,
                             vertexBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VMA_MEMORY_USAGE_CPU_ONLY);

        {
            void *dst = vertexStaging.map();
            std::memcpy(dst, allVertices.data(), static_cast<size_t>(vertexBufferSize));
            vertexStaging.unmap();
        }

        m_vertex_buffer = std::make_unique<Buffer>(m_allocator,
                                                   m_device,
                                                   vertexBufferSize,
                                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                   VMA_MEMORY_USAGE_GPU_ONLY);

        // -----------------------------
        // Index upload (staging -> GPU)
        // -----------------------------
        Buffer indexStaging(m_allocator,
                            m_device,
                            indexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VMA_MEMORY_USAGE_CPU_ONLY);

        {
            void *dst = indexStaging.map();
            std::memcpy(dst, allIndices.data(), static_cast<size_t>(indexBufferSize));
            indexStaging.unmap();
        }

        m_index_buffer = std::make_unique<Buffer>(m_allocator,
                                                  m_device,
                                                  indexBufferSize,
                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                  VMA_MEMORY_USAGE_GPU_ONLY);

        m_async_uploader.begin();

        m_async_uploader.copy_buffer(vertexStaging.handle(),
                                     m_vertex_buffer->handle(),
                                     vertexBufferSize);

        m_async_uploader.copy_buffer(indexStaging.handle(),
                                     m_index_buffer->handle(),
                                     indexBufferSize);

        UploadTicket ticket = m_async_uploader.end_and_submit();

        ANKH_LOG_DEBUG("[GpuMeshPool] Uploaded " + std::to_string(allVertices.size()) +
                       " vertices, " + std::to_string(allIndices.size()) + " indices, " +
                       std::to_string(m_draw_info.size()) + " meshes.");

        if (m_retirement)
        {
            m_retirement->retire_after(GpuSignal::timeline(ticket.value),
                                       [stagingVertexBuffer = std::move(vertexStaging),
                                        stagingIndexBuffer = std::move(indexStaging)]() mutable
                                       {
                                           // Buffers will be destroyed here when going out of scope
                                       });
        }
        else
        {
            // If no retirement queue is provided, do not return while upload is in-flight.
            // Best fallback is to block (or assert).
            ANKH_THROW_MSG("[GpuMeshPool] No retirement queue set; staging buffers will not be "
                           "freed after upload!");
        }
    }

    void GpuMeshPool::mark_used(GpuSignal signal) noexcept
    {
        if (!m_retirement)
        {
            return;
        }

        if (m_vertex_buffer)
        {
            m_vertex_buffer->set_retirement(m_retirement, signal);
        }

        if (m_index_buffer)
        {
            m_index_buffer->set_retirement(m_retirement, signal);
        }
    }

    const std::unordered_map<MeshHandle, MeshDrawInfo> &GpuMeshPool::draw_info() const noexcept
    {
        return m_draw_info;
    }

    VkBuffer GpuMeshPool::vertex_buffer() const noexcept
    {
        return m_vertex_buffer ? m_vertex_buffer->handle() : VK_NULL_HANDLE;
    }

    VkBuffer GpuMeshPool::index_buffer() const noexcept
    {
        return m_index_buffer ? m_index_buffer->handle() : VK_NULL_HANDLE;
    }

} // namespace ankh
