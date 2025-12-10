#pragma once

#include "utils/types.hpp"
#include <memory>
#include <vector>

namespace ankh
{
    struct CpuImage
    {
        int width{0};
        int height{0};
        int components{0};           // e.g. 3 = RGB, 4 = RGBA
        std::vector<uint8_t> pixels; // raw interleaved bytes
    };

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

        // base color texture (CPU-side)
        void set_base_color_image(std::shared_ptr<CpuImage> img)
        {
            m_base_color_image = std::move(img);
        }

        std::shared_ptr<const CpuImage> base_color_image() const
        {
            return m_base_color_image;
        }

        bool has_base_color_image() const
        {
            return static_cast<bool>(m_base_color_image);
        }

      private:
        glm::vec4 m_albedo{1.0f, 0.0f, 0.7f, 1.0f};
        std::shared_ptr<CpuImage> m_base_color_image; // may be null
    };

} // namespace ankh
