// src/scene/renderable.hpp
#pragma once

#include "utils/types.hpp"

namespace ankh
{
    class Mesh;
    class Material;

    using MeshHandle = uint32_t;
    using MaterialHandle = uint32_t;

    inline constexpr MeshHandle INVALID_MESH_HANDLE = 0;
    inline constexpr MaterialHandle INVALID_MATERIAL_HANDLE = 0;

    struct Renderable
    {
        MeshHandle mesh{INVALID_MESH_HANDLE};
        MaterialHandle material{INVALID_MATERIAL_HANDLE};
        glm::mat4 base_transform{1.0f};
        glm::mat4 transform{1.0f}; // model matrix
    };

} // namespace ankh
