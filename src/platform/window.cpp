#include "platform/window.hpp"
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    static void framebuffer_resize_callback(GLFWwindow *window, int, int)
    {
        auto *win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

        if (win)
        {
            win->set_framebuffer_resized(true);
        }
    }

    Window::Window(const std::string &title, uint32_t width, uint32_t height)
    {
        if (!glfwInit())
        {
            ANKH_THROW_MSG("GLFW initialization failed");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(static_cast<int>(width),
                                    static_cast<int>(height),
                                    title.c_str(),
                                    nullptr,
                                    nullptr);

        if (!m_window)
        {
            ANKH_THROW_MSG("Failed to create GLFW window");
        }

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebuffer_resize_callback);
    }

    Window::~Window()
    {
        if (m_window)
        {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        glfwTerminate();
    }

} // namespace ankh
