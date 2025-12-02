// src/scene/material.cpp
#include "scene/material.hpp"

namespace ankh
{

    Material::Material() = default;

    Material::Material(const glm::vec4 &albedo)
        : m_albedo(albedo)
    {
    }

    void Material::set_albedo(const glm::vec4 &albedo) { m_albedo = albedo; }

} // namespace ankh
