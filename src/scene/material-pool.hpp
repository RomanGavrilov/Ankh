// src/scene/material-pool.hpp
#pragma once

#include "scene/material.hpp"
#include "scene/renderable.hpp"

#include <optional>
#include <stdexcept>
#include <vector>

namespace ankh
{

    class MaterialPool
    {
      public:
        MaterialPool()
        {
            // Reserve index 0 as "invalid"
            m_materials.emplace_back(std::nullopt);
        }

        MaterialHandle create(const Material &mat)
        {
            m_materials.emplace_back(mat);
            return static_cast<MaterialHandle>(m_materials.size() - 1);
        }

        bool valid(MaterialHandle h) const
        {
            return h != INVALID_MATERIAL_HANDLE &&
                   h < static_cast<MaterialHandle>(m_materials.size()) &&
                   m_materials[h].has_value();
        }

        const Material &get(MaterialHandle h) const
        {
            if (!valid(h))
            {
                throw std::runtime_error("MaterialPool::get: invalid handle");
            }

            return *m_materials[h];
        }

        Material &get(MaterialHandle h)
        {
            if (!valid(h))
            {
                throw std::runtime_error("MaterialPool::get: invalid handle");
            }

            return *m_materials[h];
        }

      private:
        std::vector<std::optional<Material>> m_materials;
    };

} // namespace ankh
