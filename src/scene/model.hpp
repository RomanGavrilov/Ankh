#pragma once

#include "scene/renderable.hpp" // for MeshHandle / MaterialHandle
#include "utils/types.hpp"
#include <string>
#include <vector>

namespace ankh
{
    struct ModelNode
    {
        MeshHandle mesh{INVALID_MESH_HANDLE};
        MaterialHandle material{INVALID_MATERIAL_HANDLE};
        glm::mat4 local_transform{1.0f};
    };

    class Model
    {
      public:
        Model() = default;
        explicit Model(std::string source)
            : m_source_path(std::move(source))
        {
        }

        std::vector<ModelNode> &nodes()
        {
            return m_nodes;
        }
        const std::vector<ModelNode> &nodes() const
        {
            return m_nodes;
        }

        const std::string &source_path() const
        {
            return m_source_path;
        }

      private:
        std::string m_source_path;
        std::vector<ModelNode> m_nodes;
    };

} // namespace ankh
