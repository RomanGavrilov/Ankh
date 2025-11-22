#include "platform/surface.hpp"
#include <stdexcept>

namespace ankh
{

    Surface::Surface(VkInstance instance, GLFWwindow *window)
        : m_instance(instance)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &m_surface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface");
    }

    Surface::~Surface()
    {
        if (m_surface && m_instance)
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    }

} // namespace ankh
