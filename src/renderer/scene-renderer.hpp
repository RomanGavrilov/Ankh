// src/renderer/scene-renderer.hpp
#pragma once

#include "utils/types.hpp"
#include <memory>

namespace ankh
{
    class FrameContext;
    class Swapchain;
    class Camera;
    class Material;

    class SceneRenderer
    {
      public:
        SceneRenderer();
        ~SceneRenderer();

        // Update per-frame UBO (model/view/proj) and write it into FrameContext's uniform buffer.
        // 'time' is seconds since start (or any animation time).
        void update_frame(FrameContext &frame, const Swapchain &swapchain, float time);

        Camera &camera() { return *m_camera; }
        const Camera &camera() const { return *m_camera; }

      private:
        std::unique_ptr<Camera> m_camera;
        std::unique_ptr<Material> m_material;
    };

} // namespace ankh
