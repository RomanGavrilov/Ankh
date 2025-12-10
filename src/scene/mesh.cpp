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
        std::vector<Vertex> verts = {
            //      pos             normal                    color                     uv
            {{-0.5f, -0.5f, 0.0f},  {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // bottom-left
            {{0.5f, -0.5f, 0.0f},   {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // bottom-right
            {{0.5f, 0.5f, 0.0f},    {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},   // top-right
            {{-0.5f, 0.5f, 0.0f},   {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},  // top-left
        };

        std::vector<uint16_t> inds = {0, 1, 2, 2, 3, 0};

        return Mesh{std::move(verts), std::move(inds)};
    }

} // namespace ankh
