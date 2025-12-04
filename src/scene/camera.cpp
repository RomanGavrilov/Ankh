// src/scene/camera.cpp
#include "camera.hpp"

namespace ankh
{

    Camera::Camera()
    {
        // Defaults already set in member initializers
    }

    void Camera::set_position(const glm::vec3 &pos)
    {
        m_position = pos;
    }

    void Camera::set_target(const glm::vec3 &target)
    {
        m_target = target;
    }

    void Camera::set_up(const glm::vec3 &up)
    {
        m_up = up;
    }

    void Camera::set_perspective(float fovRadians, float aspect, float znear, float zfar)
    {
        m_fov = fovRadians;
        m_aspect = aspect;
        m_znear = znear;
        m_zfar = zfar;
    }

    glm::mat4 Camera::view() const
    {
        return glm::lookAt(m_position, m_target, m_up);
    }

    glm::mat4 Camera::proj() const
    {
        glm::mat4 p = glm::perspective(m_fov, m_aspect, m_znear, m_zfar);
        // GLM is OpenGL-style; Vulkan wants Y flipped:
        p[1][1] *= -1.0f;
        return p;
    }

} // namespace ankh
