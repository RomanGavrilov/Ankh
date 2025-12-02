// src/renderer/scene-renderer.cpp
#include "scene-renderer.hpp"

#include "frame/frame-context.hpp"
#include "scene/camera.hpp"
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
    }

    SceneRenderer::~SceneRenderer() = default;

    void SceneRenderer::update_frame(FrameContext &frame, const Swapchain &swapchain, float time)
    {
        UniformBufferObject ubo{};

        // Model: rotating square around Z axis, like before
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        // Camera: view + proj
        float aspect = swapchain.extent().width / static_cast<float>(swapchain.extent().height);

        m_camera->set_aspect(aspect);

        ubo.view = m_camera->view();
        ubo.proj = m_camera->proj();

        // Write into mapped UBO
        std::memcpy(frame.uniform_mapped(), &ubo, sizeof(ubo));
    }

} // namespace ankh
