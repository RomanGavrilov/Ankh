// src/scene/mesh.hpp
#pragma once
#include "utils/types.hpp"

#include <vector>

namespace ankh
{

    class Mesh
    {
      public:
        Mesh() = default;

        Mesh(std::vector<Vertex> vertices, std::vector<uint16_t> indices);

        const std::vector<Vertex> &vertices() const { return m_vertices; }
        const std::vector<uint16_t> &indices() const { return m_indices; }

        std::size_t vertex_count() const { return m_vertices.size(); }
        std::size_t index_count() const { return m_indices.size(); }

        // Create simple colored quad mesh
        static Mesh make_colored_quad();

      private:
        std::vector<Vertex> m_vertices;
        std::vector<uint16_t> m_indices;
    };

} // namespace ankh
