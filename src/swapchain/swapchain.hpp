#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class Swapchain
    {
    public:
        Swapchain();
        ~Swapchain();

        VkSwapchainKHR handle() const;
        VkFormat imageFormat() const;
        VkExtent2D extent() const;
    };

} // namespace ankh