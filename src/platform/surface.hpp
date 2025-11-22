#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class Surface
    {
    public:
        Surface(VkInstance instance, GLFWwindow *window);
        ~Surface();

        VkSurfaceKHR handle() const { return m_surface; }

    private:
        VkInstance m_instance{};
        VkSurfaceKHR m_surface{};
    };

} // namespace ankh
