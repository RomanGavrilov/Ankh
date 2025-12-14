#include "platform/surface.hpp"
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    Surface::Surface(VkInstance instance, GLFWwindow *window)
        : m_instance(instance)
    {
        ANKH_VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &m_surface));
    }

    Surface::~Surface()
    {
        if (m_surface && m_instance)
        {
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        }
    }

} // namespace ankh
