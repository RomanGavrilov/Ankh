// src/scene/renderable.hpp
#pragma once

#include "utils/types.hpp"

namespace ankh
{
    class Mesh;
    class Material;

    struct Renderable
    {
        Mesh *mesh = nullptr;
        Material *material = nullptr;
        glm::mat4 transform{1.0f}; // model matrix
    };
} // namespace ankh
