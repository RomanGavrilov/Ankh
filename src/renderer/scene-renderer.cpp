// src/renderer/scene-renderer.cpp
#include "scene-renderer.hpp"

#include "frame/frame-context.hpp"
#include "scene/camera.hpp"
#include "scene/material.hpp"
#include "swapchain/swapchain.hpp"
#include "utils/types.hpp"

#include <cstring>
#include <utils/logging.hpp>

namespace ankh
{

    SceneRenderer::SceneRenderer()
    {
        m_camera = std::make_unique<Camera>();
        // Default camera: already positioned at (2,2,2) looking at origin with fov 45°
        // Aspect will be set per-frame based on swapchain extent.

        Material defaultMaterial;
        m_default_material = m_material_pool.create(defaultMaterial);
    }

    SceneRenderer::~SceneRenderer() = default;

    void SceneRenderer::update_frame(FrameContext &, const Swapchain &swapchain, float time)
    {
        float aspect = static_cast<float>(swapchain.extent().width) /
                       static_cast<float>(swapchain.extent().height);

        m_camera->set_aspect(aspect);

        int i = 0;

        for (auto &r : m_renderables)
        {
            // rotate around Y and X so you really see depth
            glm::mat4 m = r.base_transform;
            m = glm::rotate(m, time * 0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::rotate(m, time * 0.4f, glm::vec3(1.0f, 0.0f, 0.0f));
            r.transform = m;

            ++i;
        }
    }

} // namespace ankh
