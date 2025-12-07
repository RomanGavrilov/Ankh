// src/scene/mesh-pool.hpp
#pragma once

#include "scene/mesh.hpp"
#include "scene/renderable.hpp"

#include <optional>
#include <stdexcept>
#include <vector>

namespace ankh
{

    class MeshPool
    {
      public:
        MeshPool()
        {
            // Reserve index 0 as "invalid"
            m_meshes.emplace_back(std::nullopt);
        }

        MeshHandle create(Mesh &&mesh)
        {
            m_meshes.emplace_back(std::move(mesh));
            return static_cast<MeshHandle>(m_meshes.size() - 1);
        }

        bool valid(MeshHandle h) const
        {
            return h != INVALID_MESH_HANDLE && h < static_cast<MeshHandle>(m_meshes.size()) &&
                   m_meshes[h].has_value();
        }

        const Mesh &get(MeshHandle h) const
        {
            if (!valid(h))
            {
                throw std::runtime_error("MeshPool::get: invalid handle");
            }

            return *m_meshes[h];
        }

        Mesh &get(MeshHandle h)
        {
            if (!valid(h))
                throw std::runtime_error("MeshPool::get: invalid handle");
            return *m_meshes[h];
        }

      private:
        std::vector<std::optional<Mesh>> m_meshes;
    };

} // namespace ankh
