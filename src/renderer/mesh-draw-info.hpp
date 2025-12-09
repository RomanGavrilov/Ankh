// src/renderer/mesh-draw-info.hpp
#pragma once

#include <cstdint>
#include "scene/renderable.hpp"

namespace ankh
{
    struct MeshDrawInfo
    {
        uint32_t firstIndex{0};   // first index into the unified index buffer
        uint32_t indexCount{0};   // number of indices for this mesh
        int32_t  vertexOffset{0}; // added to index as baseVertex in vkCmdDrawIndexed
    };
} // namespace ankh
