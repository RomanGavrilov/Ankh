#pragma once
#include <string>
#include "utils/Types.hpp"


namespace ankh {


class Window {
public:
Window(const std::string& title, uint32_t width, uint32_t height);
~Window();


GLFWwindow* handle() const;
void setFramebufferResized(bool v);
bool framebufferResized() const;


private:
GLFWwindow* m_window = nullptr;
bool m_framebufferResized = false;
};


} // namespace ankh