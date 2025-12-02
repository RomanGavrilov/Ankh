// src/renderer/scene-renderer.cpp
#include "scene-renderer.hpp"

#include "frame/frame-context.hpp"
#include "scene/camera.hpp"
#include "scene/material.hpp"
#include "swapchain/swapchain.hpp"
#include "utils/types.hpp"

#include <cstring>

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
        UniformBufferObject ubo{};

        // Model: rotating square around Z axis
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        // Camera: view + proj
        float aspect = swapchain.extent().width / static_cast<float>(swapchain.extent().height);

        m_camera->set_aspect(aspect);

        ubo.view = m_camera->view();
        ubo.proj = m_camera->proj();

        ubo.albedo = m_material->albedo();

        // Write into mapped UBO
        std::memcpy(frame.uniform_mapped(), &ubo, sizeof(ubo));

        // Later: we can also push material data (like albedo) to GPU here
        // via another UBO/push constant or different descriptor.
    }

} // namespace ankh
