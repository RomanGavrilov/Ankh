#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class Framebuffer
    {
    public:
        Framebuffer();
        ~Framebuffer();

        VkFramebuffer handle() const;
    };

} // namespace ankh