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
            // pos              // color
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // bottom-left  (red)
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},  // bottom-right (green)
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},   // top-right    (blue)
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},  // top-left     (white)
        };

        std::vector<uint16_t> inds = {0, 1, 2, 2, 3, 0};

        std::vector<Vertex> vertices(verts.begin(), verts.end());
        std::vector<uint16_t> indices(inds.begin(), inds.end());
        return Mesh{std::move(verts), std::move(inds)};
    }

} // namespace ankh
