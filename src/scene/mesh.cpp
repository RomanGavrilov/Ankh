// src/scene/mesh.cpp
#include "mesh.hpp"
#include "utils/types.hpp" // for Vertex, kVertices, kIndices

namespace ankh
{

    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint16_t> indices)
        : m_vertices(std::move(vertices))
        , m_indices(std::move(indices))
    {
    }

    Mesh Mesh::make_colored_quad()
    {
        // Reuse your existing static geometry for now.
        // Later you can remove kVertices/kIndices and define them here.
        std::vector<Vertex> verts(kVertices.begin(), kVertices.end());
        std::vector<uint16_t> inds(kIndices.begin(), kIndices.end());
        return Mesh{std::move(verts), std::move(inds)};
    }

} // namespace ankh
