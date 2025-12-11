// src/renderer/gpu-mesh-pool.cpp
#include "renderer/gpu-mesh-pool.hpp"

#include <cstring>
#include <utils/logging.hpp>

namespace ankh
{

    GpuMeshPool::GpuMeshPool(VkPhysicalDevice phys, VkDevice device, UploadContext &uploadContext)
        : m_phys(phys)
        , m_device(device)
        , m_upload_context(uploadContext)
    {
    }

    void GpuMeshPool::build_from_mesh_pool(const MeshPool &mesh_pool, VkQueue graphicsQueue)
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
            ANKH_LOG_WARN("GpuMeshPool::build_from_mesh_pool: no mesh data to upload.");
            m_vertex_buffer.reset();
            m_index_buffer.reset();
            return;
        }

        VkDeviceSize vertexBufferSize = sizeof(Vertex) * allVertices.size();
        VkDeviceSize indexBufferSize = sizeof(uint16_t) * allIndices.size();

        // --- Vertex staging + device-local buffer ---
        Buffer vertexStaging(m_phys,
                             m_device,
                             vertexBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            void *data = vertexStaging.map(0, vertexBufferSize);
            std::memcpy(data, allVertices.data(), static_cast<size_t>(vertexBufferSize));
            vertexStaging.unmap();
        }

        m_vertex_buffer = std::make_unique<Buffer>(m_phys,
                                                   m_device,
                                                   vertexBufferSize,
                                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_upload_context.copy_buffer(graphicsQueue,
                                     vertexStaging.handle(),
                                     m_vertex_buffer->handle(),
                                     vertexBufferSize);

        // --- Index staging + device-local buffer ---
        Buffer indexStaging(m_phys,
                            m_device,
                            indexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            void *data = indexStaging.map(0, indexBufferSize);
            std::memcpy(data, allIndices.data(), static_cast<size_t>(indexBufferSize));
            indexStaging.unmap();
        }

        m_index_buffer = std::make_unique<Buffer>(m_phys,
                                                  m_device,
                                                  indexBufferSize,
                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_upload_context.copy_buffer(graphicsQueue,
                                     indexStaging.handle(),
                                     m_index_buffer->handle(),
                                     indexBufferSize);

        ANKH_LOG_DEBUG("[GpuMeshPool] Uploaded " + std::to_string(allVertices.size()) +
                      " vertices, " + std::to_string(allIndices.size()) + " indices, " +
                      std::to_string(m_draw_info.size()) + " meshes.");
    }

} // namespace ankh
