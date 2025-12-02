#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class Camera
    {
      public:
        Camera();

        void set_position(const glm::vec3 &pos);
        void set_target(const glm::vec3 &target);
        void set_up(const glm::vec3 &up);

        void set_perspective(float fovRadians, float aspect, float znear, float zfar);

        const glm::vec3 &position() const { return m_position; }
        const glm::vec3 &target() const { return m_target; }
        const glm::vec3 &up() const { return m_up; }

        glm::mat4 view() const;
        glm::mat4 proj() const;

        // For now, projection depends on a stored aspect; you can update it on resize
        void set_aspect(float aspect) { m_aspect = aspect; }

      private:
        glm::vec3 m_position{2.0f, 2.0f, 2.0f};
        glm::vec3 m_target{0.0f, 0.0f, 0.0f};
        glm::vec3 m_up{0.0f, 0.0f, 1.0f};

        float m_fov{glm::radians(45.0f)};
        float m_aspect{4.0f / 3.0f};
        float m_znear{0.1f};
        float m_zfar{10.0f};
    };

} // namespace ankh
