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
            float speedDegrees = 10.0f + i * 30.0f; // increasing speed by index
            float speed = glm::radians(speedDegrees);

            r.transform = glm::rotate(r.base_transform, time * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));

            ++i;
        }
    }

} // namespace ankh
