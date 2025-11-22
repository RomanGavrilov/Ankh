#pragma once
#include <string>
#include "utils/types.hpp"

namespace ankh
{

    class Window
    {
    public:
        Window(const std::string &title, uint32_t width, uint32_t height);
        ~Window();

        GLFWwindow *handle() const { return m_window; }
        bool framebuffer_resized() const { return m_framebuffer_resized; }
        void set_framebuffer_resized(bool v) { m_framebuffer_resized = v; }

    private:
        GLFWwindow *m_window = nullptr;
        bool m_framebuffer_resized = false;
    };

} // namespace ankh
