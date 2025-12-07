#pragma once

#include "utils/types.hpp"

namespace ankh
{

    // Minimal material: just an albedo/tint color for now.
    // Later this can grow to hold:
    //  - one or more Textures
    //  - shader/pipeline references
    //  - roughness/metallic/etc.
    class Material
    {
      public:
        Material();
        explicit Material(const glm::vec4 &albedo);

        void set_albedo(const glm::vec4 &albedo);
        const glm::vec4 &albedo() const
        {
            return m_albedo;
        }

      private:
        glm::vec4 m_albedo{1.0f, 0.0f, 0.7f, 1.0f}; // Default magenta to spot missing albedo
    };

} // namespace ankh
