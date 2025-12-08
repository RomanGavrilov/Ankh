// src/renderer/scene-renderer.hpp
#pragma once

#include "scene/material-pool.hpp"
#include "scene/mesh-pool.hpp"
#include "scene/renderable.hpp"
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

        Camera &camera()
        {
            return *m_camera;
        }

        const Camera &camera() const
        {
            return *m_camera;
        }

        Material &material()
        {
            return m_material_pool.get(m_default_material);
        }
        const Material &material() const
        {
            return m_material_pool.get(m_default_material);
        }

        MeshPool &mesh_pool()
        {
            return m_mesh_pool;
        }

        const MeshPool &mesh_pool() const
        {
            return m_mesh_pool;
        }

        MaterialPool &material_pool()
        {
            return m_material_pool;
        }

        const MaterialPool &material_pool() const
        {
            return m_material_pool;
        }

        MaterialHandle default_material_handle() const
        {
            return m_default_material;
        }

        std::vector<Renderable> &renderables()
        {
            return m_renderables;
        }

        const std::vector<Renderable> &renderables() const
        {
            return m_renderables;
        }

      private:
        std::unique_ptr<Camera> m_camera;

        MeshPool m_mesh_pool;
        MaterialPool m_material_pool;

        MaterialHandle m_default_material;

        std::vector<Renderable> m_renderables;
    };

} // namespace ankh
