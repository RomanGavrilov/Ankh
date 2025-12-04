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

        m_material = std::make_unique<Material>(glm::vec4(0.8f, 0.2f, 0.2f, 1.0f)); // reddish
    }

    SceneRenderer::~SceneRenderer() = default;

    void SceneRenderer::update_frame(FrameContext &frame, const Swapchain &swapchain, float time)
    {
        float aspect = static_cast<float>(swapchain.extent().width) / static_cast<float>(swapchain.extent().height);

        m_camera->set_aspect(aspect);

        if (!m_renderables.empty())
        {

            Renderable &r = m_renderables[0];

            // Animate: rotate around Z
            r.transform = glm::rotate(glm::mat4(1.0f), time * glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        }
    }

} // namespace ankh
