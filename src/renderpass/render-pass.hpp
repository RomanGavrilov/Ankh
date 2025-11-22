#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class RenderPass
    {
    public:
        RenderPass();
        ~RenderPass();

        VkRenderPass handle() const;
    };

} // namespace ankh